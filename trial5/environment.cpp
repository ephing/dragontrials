#include "environment.hpp"
#include <iostream>

namespace cshanty {

Environment::Environment(const Environment& other) {
    next = nullptr;
    for (auto i : other.env) {
        const Eval* e;
        if (i.second == nullptr) e = nullptr;
        else e = i.second->clone();
        add(i.first,e);
    }
    prev = other.prev;
}

Environment::~Environment() {
    for (auto i : env) delete i.second;
    if (next != nullptr) delete next;
}

void Environment::add(std::string name,const Eval* e) { 
    env.push_back(std::pair<std::string,const Eval*>(name,e)); 
}

void Environment::set(std::string name, const Eval* e) {
    bool found = false;
    for (size_t i = env.size() - 1; i < env.size(); i--) {
        if (env.at(i).first == name) {
            if (env.at(i).second != nullptr) delete env.at(i).second;
            env.at(i) = std::pair<std::string,const Eval*>(env.at(i).first,e);
            found = true;
            break;
        }
    }
    if (!found && prev != nullptr) prev->set(name,e);
}

const Eval* Environment::find(std::string name) {
    for (size_t i = env.size() - 1; i < env.size(); i--) {
        if (env.at(i).first == name) return env.at(i).second;
    }
    if (prev != nullptr) return prev->find(name);
    return nullptr;
}

Environment* Environment::connect(Environment* e) {
    Environment* t = next;
    next = e;
    e->prev = this;
    return t;
}

void Environment::trunc(Environment* e) {
    if (next == nullptr) throw new InternalError("Attempted to remove nonexistent environment");
    delete next;
    next = e;
}

void Environment::print() const {
    std::cout << "Global Environment:\n";
    if (prev != nullptr) prev->print();
    for (auto i : env) {
        std::cout << i.first << ": ";
        if (i.second == nullptr) std::cout << "nullptr\n";
        else if (i.second->asClosure()) std::cout << "is function\n";
        else i.second->printResult();
    }
}

}