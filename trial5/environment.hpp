#ifndef CSHANTY_ENV
#define CSHANTY_ENV

#include <vector>
#include "evaluation.hpp"

namespace cshanty {

class Environment {
public:
    Environment() : prev(nullptr), next(nullptr) {}
    Environment(const Environment&);
    ~Environment();
    void add(std::string name,const Eval* e);
    void set(std::string name, const Eval* e);
    const Eval* find(std::string name);
    Environment* connect(Environment*);
    void trunc(Environment* e);
    void print() const;
private:
    std::vector<std::pair<std::string,const Eval*>> env;
    Environment* prev, *next;
};

}

#endif