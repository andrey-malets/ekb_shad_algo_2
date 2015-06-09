#include "../comb.h"
#include "../util.h"

#include <cinttypes>
#include <iostream>
#include <stack>
#include <vector>

typedef uint16_t State;
typedef uint16_t Arc;

struct Target {
  unsigned short symbol :  5;
  unsigned short state  : 11;
};

struct Transition {
  State src;
  Target dst;
};

typedef std::vector<Transition> RawTransitions;
typedef Comb<Arc, Target> Transitions;

struct Automata {
  std::vector<bool> term;
  Transitions trans;
};

unsigned char get_symbol(char raw_symbol) {
  return raw_symbol == '$' ? 0 : raw_symbol - 'a' + 1;
}

unsigned char put_symbol(char symbol) {
  assert(symbol);
  return symbol - 1 + 'a';
}

int less(const Transition lhs, const Transition rhs) {
  return lhs.src < rhs.src;
}

void read_terminals(std::istream& in, State n, State k,
                    std::vector<bool>* terminals) {
  terminals->resize(n);
  for (State i = 0; i != k; ++i) {
    State s;
    in >> s;
    (*terminals)[s] = true;
  }
}

void read_transitions(std::istream& in, State n, Arc m,
                      Transitions* transitions) {
  transitions->reserve(n, m);

  std::vector<Transition> raw_transitions(m);
  for (Arc i = 0; i != m; ++i) {
    State src, dst;
    char raw_symbol;
    in >> src >> raw_symbol >> dst;
    raw_transitions[i] = {src, {get_symbol(raw_symbol), dst}};
  }

  std::sort(raw_transitions.begin(), raw_transitions.end(), less);
  compactify_to(raw_transitions, n, transitions);
}

void read_automata(std::istream& in, Automata* a) {
  State n, k;
  Arc m;
  in >> n >> m >> k;

  read_terminals(in, n, k, &a->term);
  read_transitions(in, n, m, &a->trans);
}

void find_epsilon_components(const Automata& in, std::vector<State>* out) {
  out->resize(in.trans.rows());

  RawTransitions raw_inverted;
  raw_inverted.reserve(in.trans.size());
  for (State src = 0; src != in.trans.rows(); ++src)
    for (Arc dsti = 0; dsti != in.trans.length(src); ++dsti)
      if (in.trans.at(src, dsti).symbol == 0)
        raw_inverted.push_back({in.trans.at(src, dsti).state, {0, src}});

  std::sort(raw_inverted.begin(), raw_inverted.end(), less);

  Transitions inverted;
  compactify_to(raw_inverted, in.trans.rows(), &inverted);

  out->resize(in.trans.rows());
  for (State i = 0, comp = 0; i != out->size(); ++i) {
    while ((*out)[i] != 0 && i != out->size())
      ++i;
    if (i == out->size())
      break;
    ++comp;

    std::stack<State> to_visit;
    to_visit.push(i);
    while (!to_visit.empty()) {
      State src = to_visit.top(); to_visit.pop();
      for (Arc dsti = 0; dsti != inverted.length(src); ++dsti) {
        State dst = inverted.at(src, dst).state;
        if ((*out)[dst] == 0) {
          (*out)[dst] = comp;
          to_visit.push(dst);
        }
      }
    }
  }
}

int main(void) {
  Automata a;
  read_automata(std::cin, &a);
}
