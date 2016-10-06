#include <string>
#include <cstring>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include "astar.h"

#define INF 32767

#define START 'A'
#define FINISH 'B'
#define MARKER_PATH 'O'
#define MARKER_OPEN '*'
#define MARKER_CLOSED 'x'

char* map;
int mapWidth, mapHeight;
unordered_map<char, int> mapValues;

void getMapFromFile(string filename) {
	string filepath = "../boards/board-" + filename + ".txt";
	ifstream file(filepath);
	string rawMap = "";

	if ( file.is_open() ) {
		string line;
		int height = 0;

		while ( getline(file, line) ) {
			rawMap += line;
			mapWidth = line.length();
			height++;
		}
		mapHeight = height;
	}
	else {
		cout << "Could not open map file!" << endl;
		throw;
	}

	//Conversion from const char* to char*
	char* buffer = new char[rawMap.length() + 1];
	map = strcpy(buffer, rawMap.c_str());
	file.close();
}

int getMapValue(int x, int y) {
	if (x < 0 || y < 0 || x >= mapWidth || y >= mapHeight) {
		return -1;
	}
	return mapValues.at(map[y*mapWidth + x]);
}

void getPointFromMap(int& x, int& y, char target) {
	int i = -1;
	while ( ++i < (mapWidth*mapHeight) ) {
		if (map[i] == target) {
			y = i/mapWidth;
			x = i - y*mapWidth;
		}
	}
}

//I realise this is terrible code quality
//This is a quick work-around because MapNode depends on getMapValue(..), and I don't want it as part of an object
class MapNode {
public:
	int x,y;

	MapNode(int x, int y) : x(x), y(y) {};

	int getCostEstimate(MapNode* target) {
		return abs(target->x - x) + abs(target->y - y);
	}

	int getArcCost(MapNode* target) {
		if (getCostEstimate(target) == 1) {
			return getMapValue(target->x, target->y);
		}
		//Return invalid if nodes are not adjacent
		return -1;
	}

	bool isEqual(MapNode* other) {
		return (other->x == x) && (other->y == y);
	}

	//Adds node to successor vector for all valid cases
	void generateAllSuccessors(AStar<MapNode>* astar, SearchNode<MapNode>* parent) {
		
		//This check is necessary, as parent will be a nullptr if the node has no parent (start node)
		//Without this check, the program will segfault
		int px = astar->start->state->x;
		int py = astar->start->state->y;
		if (parent) {
			px = parent->state->x;
			py = parent->state->y;
		}
		MapNode parentNode = MapNode(py, py);

		if ( (x > 0) && (getMapValue(x-1,y) < INF)) {
			MapNode* succ = new MapNode(x-1,y);
			SearchNode<MapNode>* succNode = new SearchNode<MapNode>(succ);
			astar->successors.push_back(succNode);
		}
		if ( (x < mapWidth-1) && (getMapValue(x+1,y) < INF)) {
			MapNode* succ = new MapNode(x+1,y);
			SearchNode<MapNode>* succNode = new SearchNode<MapNode>(succ);
			astar->successors.push_back(succNode);
		}
		if ( (y > 0) && (getMapValue(x,y-1) < INF)) {
			MapNode* succ = new MapNode(x,y-1);
			SearchNode<MapNode>* succNode = new SearchNode<MapNode>(succ);
			astar->successors.push_back(succNode);
		}
		if ( (y < mapHeight-1) && (getMapValue(x,y+1)) < INF) {
			MapNode* succ = new MapNode(x,y+1);
			SearchNode<MapNode>* succNode = new SearchNode<MapNode>(succ);
			astar->successors.push_back(succNode);
		}
	}
};

//Helper function to avoid an even more messy main()
bool listContains(list<SearchNode<MapNode>*>* l, SearchNode<MapNode>* elem) {
	for (auto it = l->begin(); it != l->end(); ++it) {
		if ((*it)->state->isEqual(elem->state)) {
			return true;
		}
	}
	return false;
}

void printSolution(AStar<MapNode>& astar) {

	//Re-write the map to add closed cells
	for (auto it = astar.closedList.begin(); it != astar.closedList.end(); ++it) {
		if (!(*it)->state->isEqual(astar.start->state) && !(*it)->state->isEqual(astar.finish->state)) {
			map[(*it)->state->y * mapWidth + (*it)->state->x] = MARKER_CLOSED;
		}
	}

	//Add open cells
	for (auto it = astar.openList.begin(); it != astar.openList.end(); ++it) {
		map[(*it)->state->y * mapWidth + (*it)->state->x] = MARKER_OPEN;
	}
	
	//Add the path
	for (auto it = astar.path.begin(); it != astar.path.end(); ++it) {
		map[(*it)->y * mapWidth + (*it)->x] = MARKER_PATH;
	}

	//Print the map
	cout << endl;
	for (int y = 0; y < mapHeight; y++) {
		for (int x = 0; x < mapWidth; x++) {
			cout << map[y*mapWidth + x];
		}
		cout << endl;
	}
	cout << endl;
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		cout << "Usage: " << argv[0] << " " << "<Board number>" << endl;
		cout << "Example: " << argv[0] << " 1-1" << endl;
		return 0;
	}

	//Initialize tile costs
	mapValues['A'] = 1;
	mapValues['B'] = 1;
	mapValues['.'] = 1;
	mapValues['r'] = 1;
	mapValues['g'] = 5;
	mapValues['f'] = 10;
	mapValues['m'] = 50;
	mapValues['w'] = 100;
	mapValues['#'] = INF;

	getMapFromFile(argv[1]);

	//Find start and finish
	int startx, starty, finishx, finishy;
	getPointFromMap(startx, starty, START);
	getPointFromMap(finishx, finishy, FINISH);

	MapNode startNode(startx, starty);
	MapNode finishNode(finishx, finishy);

	AStar<MapNode> astar(&startNode, &finishNode);

	bool solved = false;

	//Agenda loop
	while (!solved) {
		if (astar.openList.empty()) {
			cout << "No solution found!" << endl;
			throw;
		}

		SearchNode<MapNode>* x = astar.agendaPop();
		astar.closedList.push_back(x);

		if (x->state->isEqual(astar.finish->state)) {
			cout << "Solution found! Total cost: " << x->g << endl;
			astar.findFinalPath(x);
			printSolution(astar);
			return 0;
		}

		x->state->generateAllSuccessors(&astar, x->parent);
		for (vector<SearchNode<MapNode>*>::iterator it = astar.successors.begin(); it != astar.successors.end(); ++it) {
			
			SearchNode<MapNode>* S = *it;
			x->children.push_back(S);

			//If S is not previously discovered
			if ( !listContains(&astar.openList, S) && !listContains(&astar.closedList, S) ) {
				astar.attachAndEval(S, x);
				astar.agendaAdd(S);
			}
			//If we found a cheaper way to S
			else if ( (x->g + x->state->getArcCost(S->state)) < S->g ) {
				astar.attachAndEval(S, x);
				if ( listContains(&astar.closedList, S) ) {
					astar.propagatePathImprovements(x);
				}
			}
		}
		//Clear contents for next iteration
		astar.successors.clear();
	}
	return 0;
}