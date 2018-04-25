#ifndef PIECEMANAGER_HPP
#define PIECEMANAGER_HPP

#include <vector>
#include <string>

class PieceManager {
public:
	PieceManager(long const numOfPieces);

private:
	std::string pieceLocation;
	std::vector<long> pieces;
	long numOfPieces;
};
#endif