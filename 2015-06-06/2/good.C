#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <queue>
#include <stack>
#include <map>
#include <set>
#include <ctime>
#include <limits>

const static char EPS = 'z'-'a' + 1;
const static char ALPH_SIZE = 'z'-'a' + 2;
typedef std::vector<std::set<unsigned int> > NDState;

class Automata{
    unsigned int _entrance;
    unsigned int _components_number;
    unsigned int _states_number;
    std::vector<NDState> _delta;
    std::vector<bool> _terminal_states;

public:
    Automata(std::istream& in) : _entrance(0){
        unsigned int m, k;
        in >> _states_number >> m >> k;
        _terminal_states.assign(_states_number, false);
        for(size_t i = 0, next_state; i < k; i++){
            in >> next_state;
            _terminal_states[next_state] = true;
        }

        unsigned int from, to;
        char transition_symbol;
        _delta.resize(_states_number, NDState(ALPH_SIZE));
        for(size_t i = 0; i < m; i++){
            in >> from >> transition_symbol >> to;
            if(transition_symbol == '$')
                transition_symbol = 'z' + 1;
            _delta[from][transition_symbol-'a'].insert(to);
        }
    }

    NDState& operator[](size_t i){
        return _delta[i];
    }

    bool isTerminal(unsigned int state) const{
        return _terminal_states[state];
    }

    unsigned int getEntrance() const{
        return _entrance;
    }

    size_t statesNumber() const{
        return _states_number;
    }

    void becomeClosure(){
        std::vector<unsigned int> degrees = compressComponents();
        addInterconnections(degrees);
    }

private:
    void addInterconnections(std::vector<unsigned int> &degree){
        std::queue<unsigned int> queue;
        for(size_t v = 0; v < _states_number; v++){
            if(degree[v] == 0)
                queue.push(v);
        }

        unsigned int v;
        bool v_terminal;
        while(!queue.empty()){
            v = queue.front();
            queue.pop();
            v_terminal = _terminal_states[v];
            if(transitionExist(_delta[v], EPS)){
                for(auto u : _delta[v][EPS]){
                    degree[u]--;
                    if(degree[u] == 0)
                        queue.push(u);

                    addNotEpsTransitions(_delta[v], _delta[u]);
                    _terminal_states[u] = (v_terminal || _terminal_states[u]);
                }

                _delta[v][EPS].clear();
            }
        }

    }

    bool inline transitionExist(NDState &transitions, unsigned int symbol){
        return transitions[symbol].size() != 0;
    }

    void addNotEpsTransitions(NDState &source, NDState &destination){
        for(size_t symbol = 0; symbol < ALPH_SIZE - 1; symbol++){
           for(auto state : source[symbol])
               destination[symbol].insert(state);
        }
    }

    void fillOrder(std::vector<unsigned int> &order,
                   std::vector<bool> &used,
                   unsigned int start){
        std::stack<unsigned int> stack;
        stack.push(start);
        used[start] = true;
        unsigned int v;
        while(!stack.empty()){
            v = stack.top();
            for(auto u : _delta[v][EPS]){
                if(!used[u]){
                    stack.push(u);
                    used[u] = true;
                }
            }
            if(stack.top() == v){
                stack.pop();
                order.push_back(v);
            }
        }
    }

    std::vector<unsigned int> getOrder(){
        std::vector<unsigned int> order;
        std::vector<bool> used(_states_number, false);

        for(size_t v = 0; v < _states_number; v++){
            if(!used[v])
                fillOrder(order, used, v);
        }

        return order;
    }

    std::vector<std::set<unsigned int>> getTransposedGraph(char symbol){
        std::vector<std::set<unsigned int>> transposed_delta(_states_number);
        for(size_t v = 0; v < _states_number; v++){
            for(auto u : _delta[v][symbol])
                transposed_delta[u].insert(v);
        }

        return transposed_delta;
    }

    void fillComponent(std::vector<std::set<unsigned int>> &transposed_delta,
                       std::vector<unsigned int> &components,
                       unsigned int component_number,
                       unsigned int start){
        std::stack<unsigned int> stack;
        stack.push(start);
        components[start] = component_number;
        while(!stack.empty()){
            unsigned int v = stack.top();
            stack.pop();
            for(auto u : transposed_delta[v]){
                if(components[u] == _states_number){
                    stack.push(u);
                    components[u] = component_number;
                }
            }
        }
    }

    std::vector<unsigned int> getComponents(){
        std::vector<unsigned int> reverse_order = getOrder();
        std::vector<std::set<unsigned int>> transposed_delta = getTransposedGraph(EPS);

        std::vector<unsigned int> components(_states_number, _states_number);
        unsigned int current_component = 0;
        unsigned int v;
        for(size_t i = 0; i < _states_number; i++){
            v = reverse_order[_states_number - i - 1];
            if(components[v] == _states_number){
                fillComponent(transposed_delta, components, current_component, v);
                current_component++;
            }
        }

        _components_number = current_component;
        return components;
    }

    std::vector<unsigned int> compressComponents(){
        std::vector<unsigned int> components = getComponents();
        std::vector<unsigned int> degrees(_states_number, 0);
        std::vector<NDState> new_delta(_components_number, NDState(ALPH_SIZE));
        for(size_t v = 0; v < _states_number; v++){
            for(size_t symbol = 0; symbol < ALPH_SIZE - 1; symbol++){
                for(auto &u : _delta[v][symbol])
                    new_delta[components[v]][symbol].insert(components[u]);
            }

            for(auto &u : _delta[v][EPS]){
                if(components[v] != components[u] &&
                    new_delta[components[u]][EPS].insert(components[v]).second)
                    degrees[components[v]]++;
            }
        }

        _entrance = components[_entrance];
        std::vector<bool> new_terminals(_components_number, false);
        for(size_t v = 0; v < _states_number; v++)
            new_terminals[components[v]] = (new_terminals[components[v]] || _terminal_states[v]);
        _terminal_states = new_terminals;

        _states_number = _components_number;
        _delta = new_delta;

        return degrees;
    }

};

class FastAutomata{
private:
	std::vector<unsigned int> _transitions;
	std::vector<bool> _is_terminal;
	size_t _states_number;
	unsigned int _entrance;
public:
	FastAutomata(Automata& automata) : _entrance(automata.getEntrance()), 
									   _states_number(automata.statesNumber()),
									   _is_terminal(_states_number){
		for(size_t i=0; i<_states_number; i++)
			_is_terminal[i] = automata.isTerminal(i);
		
		size_t data_size = _states_number * ALPH_SIZE; // place for offsets by state for each symbol from alphabet
		for(unsigned int u=0; u<_states_number; u++){
			for(unsigned int a=0; a<ALPH_SIZE; a++)
				data_size += automata[u][a].size(); // place for transitions by state and symbol
		}
		_transitions.assign(data_size, 0);

		unsigned int global_offset = _states_number * ALPH_SIZE;
		for(unsigned int state=0; state<_states_number; state++){
			for(unsigned int a=0; a<ALPH_SIZE; a++){
				_transitions[offsetFor(state, a)] = global_offset;
				for(unsigned int v : automata[state][a]){
					_transitions[global_offset] = v;
					global_offset++;
				}
			}
		}
	}

	size_t transitionsNumber(unsigned int state, unsigned int symbol) const{
		unsigned int next_offset;
		if(symbol < ALPH_SIZE - 1)
			next_offset = offsetForTransition(state, symbol+1);
		else{
			if(state < _states_number - 1)
				next_offset = offsetForTransition(state+1, 0);
			else
				next_offset = _transitions.size();
		}
		
		return next_offset - offsetForTransition(state, symbol);
	}
	
    size_t statesNumber() const{
        return _states_number;
    }

    bool isTerminal(unsigned int state) const{
        return _is_terminal[state];
    }

    unsigned int getEntrance() const{
        return _entrance;
    }

	std::vector<unsigned int>::iterator begin(unsigned int state, unsigned int symbol){
		return _transitions.begin() + offsetForTransition(state, symbol);
	}
	
	std::vector<unsigned int>::iterator end(unsigned int state, unsigned int symbol){
		return begin(state, symbol) + transitionsNumber(state, symbol);
	}

	std::vector<unsigned int> neighbors(unsigned int state, unsigned int symbol){
		return std::vector<unsigned int>(begin(state, symbol), end(state, symbol));
	}

private:
	size_t offsetForTransition(unsigned int state, unsigned int symbol)const{
		return _transitions[offsetFor(state, symbol)];
	}

	size_t offsetFor(unsigned int state, unsigned int symbol)const{
		return state * ALPH_SIZE + symbol;
	}
};

inline unsigned int length(unsigned int begin, unsigned int end){
    return end - begin + 1;
}

inline void addPair(unsigned int *states_set,
					unsigned int state,
					unsigned int index){
    if(states_set[state] > index)
        states_set[state] = index;
}

std::string getMaxSuitedSubstr(Automata& just_automata, const std::string& T){
	FastAutomata automata(just_automata);
	unsigned int max_begin = 0;
    unsigned int max_length = 0;
    size_t n = automata.statesNumber();
    unsigned int maxValue = std::numeric_limits<unsigned int>::max();
    unsigned int *current_states = new unsigned int[n];
    unsigned int *new_states = new unsigned int[n];
    unsigned int entrance = automata.getEntrance();

    memset(current_states, maxValue, n * sizeof(unsigned int));
    for(size_t i = 0, T_size = T.size(); i < T_size; i++){
        memset(new_states, maxValue, n * sizeof(unsigned int));
        addPair(current_states, entrance, i);
        for(size_t v = 0; v < n; v++){
            if(current_states[v] < T_size){
				for(auto u : automata.neighbors(v, T[i] - 'a')){
                    addPair(new_states, u, current_states[v]);
                    if(automata.isTerminal(u) && length(current_states[v], i) > max_length){
                        max_begin = current_states[v];
                        max_length = length(max_begin, i);
                    }
                }
            }
        }
        std::swap(current_states, new_states);
    }

    delete[] current_states;
    delete[] new_states;

    return T.substr(max_begin, max_length);
}

int main(){
    Automata automata(std::cin);
    std::string str;
    std::getline(std::cin, str);
    std::getline(std::cin, str);

    automata.becomeClosure();
    std::string answer = getMaxSuitedSubstr(automata, str);
    std::cout << (answer.size() == 0 ? "No solution" : answer.c_str()) << std::endl;

    return 0;
}