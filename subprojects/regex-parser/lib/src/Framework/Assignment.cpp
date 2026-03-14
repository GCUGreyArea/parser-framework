#include <iostream>

#include "Assignment.h"

std::shared_ptr<Assignment> Assignment::build(std::vector<std::shared_ptr<Variable>>& var, std::vector<Assignment::Operator>& ops) {
    // Reverse the order of the lists
    size_t size = var.size();

    if(size < 2)
        throw Exception::General("Not enough variables for assignment in Assignment::build");

    if(size-1 != ops.size())
        throw Exception::General("Operators unbalanced in assignment in build");

    std::shared_ptr<Assignment> ptr = nullptr;
    for(int idx=size-1;idx>0;idx--) {
        if(idx<0)
            break;
        if(ptr == nullptr) {
            auto& v2 = var[idx--];
            auto& op = ops[idx];
            auto& v1 = var[idx];

            ptr = std::make_shared<Assignment>(v1,op,v2);
        }
        else {
            auto& v1 = var[idx];
            auto& op = ops[idx];

            ptr = std::make_shared<Assignment>(v1,op,ptr);
        }
    }

    if(size > 2) {
        auto& v1 = var[0];
        auto& op = ops[0];

        ptr = std::make_shared<Assignment>(v1,op,ptr);
    }

    return ptr;

}

