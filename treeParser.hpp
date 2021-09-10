#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <vector>

enum class relationtype {LE, GT};  // less-and-equal or greater-than

class treeEdge
{
private:
  relationtype r;
  float threshold;
public:
  treeEdge(relationtype relation, float threshold):threshold{threshold},r{relation}{};
  float getThreshold() {return threshold;};
  std::string getRelation();
};

class treeNode
{
private:
  std::string feature;
  int label;
  bool isLeaf;
  std::vector<treeEdge> edges;
  std::vector<treeNode*> children;

public:
  treeNode() = default;  //default constructor
  treeNode(const std::string feature, treeEdge edge):feature{feature}, isLeaf{false}{
    edges.push_back(edge);
  }; // constructor with string input
  treeNode(int label): label{label},isLeaf{true}{};

  bool isLeafNode(){return isLeaf;};

  std::string getFeature(){return feature;};
  int getLabel(){return (isLeafNode())?label:-999;};
  std::vector<treeNode*>& getChildren(){return children;};
  std::vector<treeEdge>& getEdges(){return edges;};

  void addEdge(treeEdge edge){edges.push_back(edge);};
  void addChild(treeNode* node){children.push_back(node);};
  void addLeafNode(int label);
};

void printTree(treeNode* root);
