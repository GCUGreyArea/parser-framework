#include <iostream>
#include <stack>

#include <cctype>
#include <cstdio>
#include <cstring>
#include <string_view>
#include <utility>

#include "JQ.hpp"
#include "Numbers.h"
#include "hash.h"
#include "jsmn.hpp"
#include "kv_state.h"

namespace {

path_key make_path_key(unsigned int hash, unsigned int depth) {
    return path_key{hash, depth};
}

bool validate_json_fragment(std::string_view value, jsmntype_t type) {
    if (value.empty() || type == JSMN_UNDEFINED) {
        return false;
    }

    std::string fragment(value);
    jsmn_parser parser(fragment.c_str());
    if (parser.parse() < 0 || parser.last_token() == 0) {
        return false;
    }

    return parser.get_token(0)->type == type;
}

std::string build_insert_candidate(const std::string &json, int insert_at,
                                   std::string_view prefix,
                                   std::string_view value,
                                   bool container_is_empty) {
    std::string candidate;
    candidate.reserve(json.size() + prefix.size() + value.size() + 2);
    candidate.append(json.data(), static_cast<size_t>(insert_at));
    candidate.append(prefix.data(), prefix.size());
    candidate.append(value.data(), value.size());
    if (!container_is_empty) {
        candidate.push_back(',');
    }
    candidate.append(json.data() + insert_at,
                     json.size() - static_cast<size_t>(insert_at));
    return candidate;
}

} // namespace

jsmn_parser::~jsmn_parser() {
    if (m_tokens) {
        delete[] m_tokens;
    }

    m_tokens = nullptr;
    m_num_tokens = 0;
    m_length = 0;
}

jsmntok_t *jsmn_parser::jsmn_alloc_token() {
    jsmntok_t *tok;

    // Over the allocated size, realloc.
    if (m_token_next >= m_num_tokens) {
        jsmntok_t *prev = m_tokens;
        unsigned int num_tokens = m_num_tokens;
        m_num_tokens = m_num_tokens * m_mull;
        jsmntok_t *next_toks = new jsmntok_t[m_num_tokens];
        std::memcpy(next_toks, prev, sizeof(jsmntok_t) * (size_t)num_tokens);

        delete[] m_tokens;
        m_tokens = next_toks;
    }

    // ASsign the token and return
    tok = &m_tokens[m_token_next++];
    tok->start = tok->end = -1;
    tok->size = 0;
#ifdef JSMN_PARENT_LINKS
    tok->parent = -1;
#endif
    return tok;
}

/**
 * Fills token type and boundaries.
 */
void jsmn_parser::jsmn_fill_token(jsmntok_t *token, const int start,
                                  const int end, jsmntype_t type) {
    token->type = type;
    token->start = start;
    token->end = end;
    token->size = 0;
}

/**
 * Fills next available token with JSON primitive.
 */
int jsmn_parser::jsmn_parse_primitive() {
    jsmntok_t *token;
    int start;

    start = m_pos;

    for (; m_pos < m_length && m_js[m_pos] != '\0'; m_pos++) {
        switch (m_js[m_pos]) {
#ifndef JSMN_STRICT
        /* In strict mode primitive must be followed by "," or "}" or "]" */
        case ':':
#endif
        case '\t':
        case '\r':
        case '\n':
        case ' ':
        case ',':
        case ']':
        case '}':
            goto found;
        default:
            /* to quiet a warning from gcc*/
            break;
        }
        if (m_js[m_pos] < 32 || m_js[m_pos] >= 127) {
            m_pos = start;
            return JSMN_ERROR_INVAL;
        }
    }
#ifdef JSMN_STRICT
    /* In strict mode primitive must be followed by a comma/object/array */
    m_pos = start;
    return JSMN_ERROR_PART;
#endif

found:
    if (m_tokens == NULL) {
        m_pos--;
        return 0;
    }
    token = jsmn_alloc_token();
    if (token == NULL) {
        m_pos = start;
        return JSMN_ERROR_NOMEM;
    }
    jsmn_fill_token(token, start, m_pos, JSMN_PRIMITIVE);
#ifdef JSMN_PARENT_LINKS
    token->parent = m_toksuper;
#endif
    m_pos--;
    return 0;
}

/**
 * Fills next token with JSON string.
 */
int jsmn_parser::jsmn_parse_string() {
    jsmntok_t *token;

    int start = m_pos;

    /* Skip starting quote */
    m_pos++;

    for (; m_pos < m_length && m_js[m_pos] != '\0'; m_pos++) {
        char c = m_js[m_pos];

        /* Quote: end of string */
        if (c == '\"') {
            if (m_tokens == NULL) {
                return 0;
            }
            token = jsmn_alloc_token();
            if (token == NULL) {
                m_pos = start;
                return JSMN_ERROR_NOMEM;
            }
            jsmn_fill_token(token, start + 1, m_pos, JSMN_STRING);
#ifdef JSMN_PARENT_LINKS
            token->parent = m_toksuper;
#endif
            return 0;
        }

        /* Backslash: Quoted symbol expected */
        if (c == '\\' && m_pos + 1 < m_length) {
            int i;
            m_pos++;
            switch (m_js[m_pos]) {
            /* Allowed escaped symbols */
            case '\"':
            case '/':
            case '\\':
            case 'b':
            case 'f':
            case 'r':
            case 'n':
            case 't':
                break;
            /* Allows escaped symbol \uXXXX */
            case 'u':
                m_pos++;
                for (i = 0; i < 4 && m_pos < m_length && m_js[m_pos] != '\0';
                     i++) {
                    /* If it isn't a hex character we have an error */
                    if (!((m_js[m_pos] >= 48 && m_js[m_pos] <= 57) || /* 0-9 */
                          (m_js[m_pos] >= 65 && m_js[m_pos] <= 70) || /* A-F */
                          (m_js[m_pos] >= 97 &&
                           m_js[m_pos] <= 102))) { /* a-f */
                        m_pos = start;
                        return JSMN_ERROR_INVAL;
                    }
                    m_pos++;
                }
                m_pos--;
                break;
            /* Unexpected symbol */
            default:
                m_pos = start;
                return JSMN_ERROR_INVAL;
            }
        }
    }
    m_pos = start;
    return JSMN_ERROR_PART;
}

/**
 * Parse JSON string and fill m_tokens.
 */
int jsmn_parser::parse() {
    int r;
    int i;
    jsmntok_t *token;
    int count = m_token_next;

    m_paths.clear();

    for (; m_pos < m_length && m_js[m_pos] != '\0'; m_pos++) {
        char c;
        jsmntype_t type;

        c = m_js[m_pos];
        switch (c) {
        case '{':
        case '[':
            count++;
            // m_depth++;
            if (m_tokens == NULL) {
                break;
            }
            token = jsmn_alloc_token();
            if (token == NULL) {
                return JSMN_ERROR_NOMEM;
            }
            if (m_toksuper != -1) {
                jsmntok_t *t = &m_tokens[m_toksuper];
#ifdef JSMN_STRICT
                /* In strict mode an object or array can't become a key */
                if (t->type == JSMN_OBJECT) {
                    return JSMN_ERROR_INVAL;
                }
#endif
                t->size++;
#ifdef JSMN_PARENT_LINKS
                token->parent = m_toksuper;
#endif
            }
            token->type = (c == '{' ? JSMN_OBJECT : JSMN_ARRAY);
            token->start = m_pos;
            m_toksuper = m_token_next - 1;
            break;
        case '}':
        case ']':
            // m_depth--;
            if (m_tokens == NULL) {
                break;
            }
            type = (c == '}' ? JSMN_OBJECT : JSMN_ARRAY);
#ifdef JSMN_PARENT_LINKS
            if (m_token_next < 1) {
                return JSMN_ERROR_INVAL;
            }
            token = &m_tokens[m_token_next - 1];
            for (;;) {
                if (token->start != -1 && token->end == -1) {
                    if (token->type != type) {
                        return JSMN_ERROR_INVAL;
                    }
                    token->end = m_pos + 1;
                    m_toksuper = token->parent;
                    break;
                }
                if (token->parent == -1) {
                    if (token->type != type || m_toksuper == -1) {
                        return JSMN_ERROR_INVAL;
                    }
                    break;
                }
                token = &m_tokens[token->parent];
            }
#else
            for (i = m_token_next - 1; i >= 0; i--) {
                token = &m_tokens[i];
                if (token->start != -1 && token->end == -1) {
                    if (token->type != type) {
                        return JSMN_ERROR_INVAL;
                    }
                    m_toksuper = -1;
                    token->end = m_pos + 1;
                    break;
                }
            }
            /* Error if unmatched closing bracket */
            if (i == -1) {
                return JSMN_ERROR_INVAL;
            }
            for (; i >= 0; i--) {
                token = &m_tokens[i];
                if (token->start != -1 && token->end == -1) {
                    m_toksuper = i;
                    break;
                }
            }
#endif
            break;
        case '\"':
            r = jsmn_parse_string();
            if (r < 0) {
                return r;
            }
            count++;
            if (m_toksuper != -1 && m_tokens != NULL) {
                m_tokens[m_toksuper].size++;
            }
            break;
        case '\t':
        case '\r':
        case '\n':
        case ' ':
            break;
        case ':':
            m_toksuper = m_token_next - 1;
            break;
        case ',':
            if (m_tokens != NULL && m_toksuper != -1 &&
                m_tokens[m_toksuper].type != JSMN_ARRAY &&
                m_tokens[m_toksuper].type != JSMN_OBJECT) {
#ifdef JSMN_PARENT_LINKS
                m_toksuper = m_tokens[m_toksuper].parent;
#else
                for (i = m_token_next - 1; i >= 0; i--) {
                    if (m_tokens[i].type == JSMN_ARRAY ||
                        m_tokens[i].type == JSMN_OBJECT) {
                        if (m_tokens[i].start != -1 && m_tokens[i].end == -1) {
                            m_toksuper = i;
                            break;
                        }
                    }
                }
#endif
            }
            break;
#ifdef JSMN_STRICT
        /* In strict mode primitives are: numbers and booleans */
        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case 't':
        case 'f':
        case 'n':
            /* And they must not be keys of the object */
            if (m_tokens != NULL && m_toksuper != -1) {
                const jsmntok_t *t = &m_tokens[m_toksuper];
                if (t->type == JSMN_OBJECT ||
                    (t->type == JSMN_STRING && t->size != 0)) {
                    return JSMN_ERROR_INVAL;
                }
            }
#else
        /* In non-strict mode every unquoted value is a primitive */
        default:
#endif
            r = jsmn_parse_primitive();
            if (r < 0) {
                return r;
            }
            count++;
            if (m_toksuper != -1 && m_tokens != NULL) {
                m_tokens[m_toksuper].size++;
            }
            break;

#ifdef JSMN_STRICT
        /* Unexpected char in strict mode */
        default:
            return JSMN_ERROR_INVAL;
#endif
        }
    }

    if (m_tokens != NULL) {
        for (i = m_token_next - 1; i >= 0; i--) {
            /* Unmatched opened object or array */
            if (m_tokens[i].start != -1 && m_tokens[i].end == -1) {
                return JSMN_ERROR_PART;
            }
        }
    }

    m_paths_dirty = true;

    return count;
}

/**
 * Creates a new parser based over a given buffer with an array of m_tokens
 * available.
 */
void jsmn_parser::init(const char *js) {
    m_pos = 0;
    m_token_next = 0;
    m_toksuper = -1;

    m_paths.clear();
    m_paths_dirty = true;
    m_js = js;
    m_length = m_js.length();
    std::memset(m_tokens, 0, sizeof(jsmntok_t) * m_num_tokens);
}

void jsmn_parser::init(std::string js) {
    init(js.c_str());
}

bool jsmn_parser::serialise(const char *file_name) {
    FILE *fp = fopen(file_name, "wd");
    if (fp == NULL) {
        return false;
    }

    // Save the string and length
    fwrite(&m_length, sizeof(m_length), 1, fp);
    fwrite(m_js.data(), sizeof(char), m_length, fp);
    fwrite(&m_pos, sizeof(m_pos), 1, fp);

    // Write out the last token index
    fwrite(&m_token_next, sizeof(m_token_next), 1, fp);

    // Write the number of objects first
    fwrite(m_tokens, sizeof(struct jsmntok), m_token_next, fp);

    // Close the file and exit
    fclose(fp);

    return true;
}

/**
 * @brief Deserialise a previously serialised JSON structure
 *
 * @param file_name
 * @return true
 * @return false
 */
bool jsmn_parser::deserialise(const char *file_name) {
    m_paths.clear();
    m_paths_dirty = true;

    FILE *fp = fopen(file_name, "rd");
    if (fp == NULL) {
        return false;
    }

    // Retrieve the string
    size_t size = fread(&m_length, sizeof(m_length), 1, fp);
    if (m_length == 0 || size == 0) {
        return false;
    }

    // Read in the string
    char *js = new char[m_length + 1];
    size = fread(js, sizeof(char), m_length, fp);
    if (size == 0) {
        goto error;
    }
    js[m_length] = '\0';

    m_js = js;
    delete[] js;

    // Read back the position
    size = fread(&m_pos, sizeof(m_pos), 1, fp);
    if (size == 0) {
        goto error;
    }

    // Read in the last token idex
    size = fread(&m_token_next, sizeof(m_token_next), 1, fp);
    if (size == 0) {
        goto error;
    }

    // Set up the token array if we need to
    if ((m_token_next - 1) > m_num_tokens) {
        if (m_tokens != nullptr) {
            delete[] m_tokens;
        }
        m_tokens = new jsmntok_t[m_token_next + 1];
        m_num_tokens = m_token_next + 1;
    }

    size = fread(m_tokens, sizeof(struct jsmntok), m_token_next, fp);
    if (size == 0) {
        goto error;
    }

    fclose(fp);

    return true;

error:
    fclose(fp);
    return false;
}

void jsmn_parser::print_token(int idx) {
    jsmntok_t *t = &m_tokens[idx];

    switch (t->type) {
    case JSMN_UNDEFINED:
        std::cout << "type: undefined, ";
        break;
    case JSMN_OBJECT:
        std::cout << "type: object, ";
        break;
    case JSMN_ARRAY:
        std::cout << "type: array, ";
        break;
    case JSMN_STRING:
        std::cout << "type: string, ";
        break;
    case JSMN_PRIMITIVE:
        std::cout << "type: primitive, ";
        break;
    }

    std::cout << "start : " << t->start << ", ";
    std::cout << "end : " << t->end << ", ";
    std::cout << "size : " << t->size << ", ";
    std::cout << "parent : " << t->parent << ", ";

    // copy the string into a buffer and print it
    if (t->type == JSMN_STRING || t->type == JSMN_PRIMITIVE) {
        size_t len = t->end - t->start;
        std::string str(m_js.data() + t->start, len);
        std::cout << "value: " << "\"" << str << "\"" << std::endl;
    } else {
        std::cout << std::endl;
    }
}

/**
 * @brief Render a set of tokens into a set of jq paths
 * The design of this detailed in [Render design](./docs/render-design.md)
 *
 * @param depth
 * @param token
 */

void jsmn_parser::render(int depth, unsigned int hash_value,
                         unsigned int &token) {
    if ((int)token >= m_token_next) {
        return;
    }

    const unsigned int current = token;
    switch (m_tokens[current].type) {
    case JSMN_OBJECT: {
        const int end = m_tokens[current].end;
        token++;

        while ((int)token < m_token_next && m_tokens[token].start < end) {
            if (m_tokens[token].parent != (int)current) {
                token++;
                continue;
            }

            if (m_tokens[token].type != JSMN_STRING) {
                throw std::runtime_error("unexpected object child token type");
            }

            const int key_len = m_tokens[token].end - m_tokens[token].start;
            const char *key = m_js.c_str() + m_tokens[token].start;
            unsigned int child_hash =
                merge_hash(hash_value, hash(key, static_cast<size_t>(key_len)));

            unsigned int value_token = token + 1;
            if ((int)value_token >= m_token_next) {
                throw std::runtime_error("missing object value token");
            }

            JQ path(depth + 1, child_hash, &m_tokens[value_token], m_js.c_str());
            m_paths.emplace(make_path_key(child_hash, depth + 1), path);

            token = value_token;
            if (m_tokens[token].type == JSMN_OBJECT ||
                m_tokens[token].type == JSMN_ARRAY) {
                render(depth + 1, child_hash, token);
            } else {
                token++;
            }
        }
        break;
    }

    case JSMN_ARRAY: {
        const int end = m_tokens[current].end;
        unsigned int index = 0;
        token++;

        while ((int)token < m_token_next && m_tokens[token].start < end) {
            if (m_tokens[token].parent != (int)current) {
                token++;
                continue;
            }

            char element[32];
            int len = std::snprintf(element, sizeof(element), "[%u]", index++);
            unsigned int child_hash =
                merge_hash(hash_value, hash(element, static_cast<size_t>(len)));

            JQ path(depth + 1, child_hash, &m_tokens[token], m_js.c_str());
            m_paths.emplace(make_path_key(child_hash, depth + 1), path);

            if (m_tokens[token].type == JSMN_OBJECT ||
                m_tokens[token].type == JSMN_ARRAY) {
                render(depth + 1, child_hash, token);
            } else {
                token++;
            }
        }
        break;
    }

    case JSMN_STRING:
    case JSMN_PRIMITIVE: {
        JQ path(depth, hash_value, &m_tokens[token], m_js.c_str());
        m_paths.emplace(make_path_key(hash_value, depth), path);
        token++;
        break;
    }

    case JSMN_UNDEFINED:
        throw std::runtime_error("undefined token type");
    }
}

void jsmn_parser::rebuild_paths() {
    m_paths.clear();
    if (m_token_next == 0) {
        m_paths_dirty = false;
        return;
    }

    m_paths.reserve(static_cast<size_t>(m_token_next));

    unsigned int token = 0;
    render(0, 0, token);
    m_paths_dirty = false;
}

void jsmn_parser::ensure_paths() {
    if (!m_paths_dirty) {
        return;
    }

    rebuild_paths();
}

void jsmn_parser::render() {
    ensure_paths();
}

JQ *jsmn_parser::get_path(struct jqpath *p) {
    if (p) {
        ensure_paths();
        auto it = m_paths.find(make_path_key(p->hash, p->depth));
        if (it != m_paths.end()) {
            return &it->second;
        }
    }

    return nullptr;
}

void jsmn_parser::swap_state(jsmn_parser &other) {
    using std::swap;

    swap(m_pos, other.m_pos);
    swap(m_token_next, other.m_token_next);
    swap(m_toksuper, other.m_toksuper);
    swap(m_tokens, other.m_tokens);
    swap(m_num_tokens, other.m_num_tokens);
    swap(m_js, other.m_js);
    swap(m_length, other.m_length);
    swap(m_mull, other.m_mull);
    swap(m_depth, other.m_depth);
    swap(m_paths_dirty, other.m_paths_dirty);
    swap(m_paths, other.m_paths);
}

// size shows keys + values
int jsmn_parser::find_key_in_object(int tkn, std::string search) {

    if(tkn >= m_token_next) {
        throw std::runtime_error("Token out of bounds : " + std::to_string(tkn));
    }

    if(m_tokens[tkn].type != JSMN_OBJECT) {
        throw std::runtime_error("Token is not an object : " + std::to_string(tkn));
    }

    int place = tkn+1;

    int i = place;
    int end = m_tokens[tkn].end;
    while(i < m_token_next && m_tokens[i].start < end) {
        // Only test tokens who's parent is the object
        if(m_tokens[i].parent == tkn) {
            std::string st = extract_string(i);
            if(st == search) {
                return i;
            }
        }

        int end_this = m_tokens[i].end;
        while(i < m_token_next && m_tokens[i].start < end_this) {
            i++;
        }
    }

    return -1;
}

bool jsmn_parser::is_object(int tkn) {
    if(tkn >= m_token_next) {
        return false;
    }

    return m_tokens[tkn].type == JSMN_OBJECT;
}

bool jsmn_parser::is_string(int tkn) {
    if(tkn >= m_token_next) {
        return false;
    }

    return m_tokens[tkn].type == JSMN_STRING;    
}

std::string jsmn_parser::extract_string(int idx) {
    if(idx >= m_token_next) {
        throw std::runtime_error("token is out of bounds : " + std::to_string(idx));
    }

    jsmntok_t *t = &m_tokens[idx];
    size_t len = t->end - t->start;
    return std::string(m_js.data() + t->start, len);
}

/**
 * @brief 
 * 
 * @param key The key to use in the JSON object for this value 
 * @param value The value to insert
 * @param type The JSMN type for the object
 * @param obj The offset in the tokens that represents the object into which the key/value will be inserted 
 * @return true 
 * @return false 
 */
bool jsmn_parser::inster_kv_into_obj(std::string key, std::string value, jsmntype_t type, int obj) {
    if(obj >= m_token_next) {
        return false;
    }

    if(m_tokens[obj].type != JSMN_OBJECT) {
        return false;
    }

    jsmntok_t *t = &m_tokens[obj];

    // Key must be of type string and must start with and end with "
    if(key.length() < 2 || key[0] != '"' || key[key.length()-1] != '"') {
        return false;
    }

    if(!validate_json_fragment(value, type)) {
        return false;
    }

    std::string prefix = key + ":";
    std::string json = build_insert_candidate(m_js, t->start + 1, prefix, value,
                                              t->size == 0);
    jsmn_parser next(json.c_str(), m_mull);
    if(next.parse() < 0) {
        throw std::runtime_error("invalid JSON after insert : " + json);
    }

    swap_state(next);
    return true;
}

bool jsmn_parser::insert_value_in_list(std::string value, jsmntype_t type, int obj) {

    if(obj >= m_token_next) {
        return false;
    }

    if(m_tokens[obj].type != JSMN_ARRAY) {
        return false;
    }

    if(!validate_json_fragment(value, type)) {
        return false;
    }

    jsmntok_t *t = &m_tokens[obj];
    std::string json = build_insert_candidate(m_js, t->start + 1, "", value,
                                              t->size == 0);
    jsmn_parser next(json.c_str(), m_mull);
    if(next.parse() < 0) {
        throw std::runtime_error("invalid JSON after insert : " + json);
    }

    swap_state(next);
    return true;
}

bool jsmn_parser::update_value_for_key(int k, std::string v) {
    if(k >= m_token_next || k+1 >= m_token_next) {
        throw std::runtime_error("Token out of bounds : " + std::to_string(k));
    }

    jsmntok_t * val = &m_tokens[k+1];

    std::string start(m_js.data(), val->start);
    std::string end(m_js.data() + val->end,m_js.length()-val->end);

    std::string json = start + v + end;
    jsmn_parser next(json.c_str(), m_mull);
    if(next.parse() < 0) {
        throw std::runtime_error("JSON failed to parse : " + json);
    }

    swap_state(next);
    return true;
}
