#ifndef DYNAMIC_PROPERTY_COMPILER_H
#define DYNAMIC_PROPERTY_COMPILER_H

#include <map>
#include <memory>
#include <string>

#include "Ast.h"
#include "Framework/ConditionalAssignment.h"
#include "Framework/Token.h"

namespace DynamicProperty {

class Driver {
public:
    explicit Driver(std::string input);
    ~Driver();

    ConditionalAstPtr parse();

    void set_result(ConditionalAstPtr ast) {
        mResult = std::move(ast);
    }

    const std::string& input() const {return mInput;}
    const std::string& error() const {return mError;}
    void set_error(std::string error) {mError = std::move(error);}

    void* scanner() const {return mScanner;}

private:
    std::string mInput;
    std::string mError;
    ConditionalAstPtr mResult;
    void* mScanner;
};

std::shared_ptr<ConditionalAssignment> compile(const std::string& source, const std::map<std::string, std::shared_ptr<Token>>& token_map);

} // namespace DynamicProperty

#endif//DYNAMIC_PROPERTY_COMPILER_H
