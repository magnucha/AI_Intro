#pragma once
#include <vector>
#include <list>

//Only keep one mode active at the same time
//#define MODE_ASTAR
//#define MODE_BFS
#define MODE_DIJKSTRA

using namespace std;

/*
The following are template classes because I didn't want everything in the same file.
As these two depend on class MapNode (that had to be defined in main.cpp), and forward declarations of class MapNode didn't work, 
these two ended up as template classes to avoid undefined type error.
*/

template <class NodeState_t>
class SearchNode{
public:
	int g, h, f;

	SearchNode* parent;
	std::vector<SearchNode*> children;
	NodeState_t* state;

	SearchNode() : g(0), h(0), f(0), parent(0) {
	}

	SearchNode(NodeState_t* state) : SearchNode() {
		this->state = state;
	}

	~SearchNode() {
		delete parent;
	}
};

template <class NodeState_t>
class AStar {
public:
	SearchNode<NodeState_t>* start;
	SearchNode<NodeState_t>* finish;
	vector<NodeState_t*> path;

	std::vector<SearchNode<NodeState_t>*> successors;
	std::list<SearchNode<NodeState_t>*> openList;
	std::list<SearchNode<NodeState_t>*> closedList;

	AStar(NodeState_t* startPoint, NodeState_t* finishPoint) {
		start = new SearchNode<NodeState_t>();
		finish = new SearchNode<NodeState_t>();

		start->state = startPoint;
		finish->state = finishPoint;

		start->h = start->state->getCostEstimate(finish->state);
		start->f = start->g + start->h;

		openList.push_back(start);
	}

	~AStar() {
		delete start;
		delete finish;
	}

	void attachAndEval(SearchNode<NodeState_t>* child, SearchNode<NodeState_t>* parent) {
		if (parent->state->getArcCost(child->state) < 0) {
			cout << "\n\n Trying to attach non-adjacent MapNodes!" << endl;
			cout << "Parent: (" << parent->state->x << "," << parent->state->y << ")" << endl;
			cout << "Child: (" << child->state->x << "," << child->state->y << ")" << endl;
			throw;
		}
		child->parent = parent;
		parent->children.push_back(child);

		child->g = parent->g + parent->state->getArcCost(child->state);
		child->h = child->state->getCostEstimate(finish->state);
		child->f = child->g + child->h;
	}

	//Note that childIt is not the child SearchNode object, *childIt is
	void propagatePathImprovements(SearchNode<NodeState_t>* parent) {
		for (auto childIt = parent->children.begin(); childIt != parent->children.end(); ++childIt) {
			if ( (parent->g + parent->state->getArcCost((*childIt)->state)) < (*childIt)->g ) {
				(*childIt)->parent = parent;
				(*childIt)->g = parent->g + parent->state->getArcCost((*childIt)->state);
				(*childIt)->f = (*childIt)->g + (*childIt)->h;
				propagatePathImprovements(*childIt);
			}
		}
	}

	//Behavior of the function is defined by which mode is activated at the top of the file
	void agendaAdd(SearchNode<NodeState_t>* node) {
		#ifdef MODE_ASTAR
			auto it = openList.begin();

			//Iterate over the agenda, until a suitable insertion spot is found
			while( (it != openList.end()) && ((*it)->f < node->f )) {
				++it;
			}
			openList.insert(it, node);
		#endif

		#ifdef MODE_BFS
			openList.push_back(node);
		#endif

		#ifdef MODE_DIJKSTRA
			auto it = openList.begin();

			//Iterate over the agenda, until a suitable insertion spot is found
			while( (it != openList.end()) && ((*it)->g < node->g )) {
				++it;
			}
			openList.insert(it, node);
		#endif
	}

	SearchNode<NodeState_t>* agendaPop() {
		if (!openList.empty()) {
			SearchNode<NodeState_t>* ret = openList.front();
			openList.pop_front();
			return ret;
		}
		return nullptr;
	}

	//Iterate from the back to the starting point, as each node in the path only has 1 (optimal) parent
	void findFinalPath(SearchNode<NodeState_t>* finish) {
		SearchNode<NodeState_t>* x = finish->parent;
		while (!x->state->isEqual(start->state)) {
			path.push_back(x->state);
			x = x->parent;
		}
	}
};


