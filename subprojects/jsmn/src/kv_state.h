#ifndef KV_STATE
#define KV_STATE

enum class kv_state {
    START = 0,
    KEY   = 1,
    VALUE = 2,
    NEXT  = 3
};

inline kv_state& operator++(kv_state& state, int dummy) {

    if(state == kv_state::NEXT) {
        state = kv_state::START;
        return state;
    }

	state = static_cast<kv_state>(static_cast<int>(state) + 1);
    return state;
}

#endif // ! KV_STATE