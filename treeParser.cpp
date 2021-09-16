#include "treeParser.hpp"
#include "csvfile.h"


bool debug = true;

using namespace std;


// get the relation in string format
string treeEdge::getRelation()
{
  relationtype r = this->r;
  string ret;
  switch (r)
  {
    case relationtype::LE:
      ret = "<=";
      break;
    case relationtype::GT:
      ret = ">";
      break;
  };
  return ret;
}


/* Adding the decision as leaf node.  
   Note: I am reusing the node class to represent leaf node as well. The leaf node has only label, but no feature associated with it.
*/

void treeNode::addLeafNode(int label, float numObserved, float numMisClassfied)
{
  treeNode* newNode = new treeNode(label, numObserved, numMisClassfied);
  this->addChild(newNode);
}

void insertMap(treeNode* node, std::unordered_map<treeNode*, int>& mymap)
{
  static int idx;
  if(mymap.find(node) == mymap.end())
  {
    ++idx;
    mymap.insert({node, idx});
  }

}

// Print function to print out tree content to reproduce the text from the input text file.

void printCSV(treeNode* root, csvfile& csv, int treeLevel, unordered_map<treeNode*, int>& mymap)
{
  if(root == nullptr)
    return;
  insertMap(root, mymap);
  bool hasChild = !(root->isLeafNode());
  vector<treeEdge> edges = root->getEdges();
  vector<treeNode*> children = root->getChildren();
  // the edge and child node are loosely coupled now, check the numbers before printing.   

  if(edges.size() != children.size())
  {
    cerr<< "edges and children nodes number mismatch" << endl;
    return;
  } 
  csv << mymap.find(root)->second << treeLevel << root->getFeature();

  if(root->isLeafNode())
  {
    // print label
    csv << root->getLabel();
    // print has child and relationOp
    csv << hasChild << "";
    // print empty cell for relationVal
    csv.addEmptyCell(); 
    // print empty cell for trueNode
    csv.addEmptyCell(); 
    // print empty cell for falseNode
    csv.addEmptyCell(); 
    csv << endrow;
  }
  else
  {
    csv.addEmptyCell(); 
    //string relationstr = edges[0].getRelation() + to_string(edges[0].getThreshold()); 
    //csv << hasChild << relationstr ;
    csv << hasChild << edges[0].getRelation() << edges[0].getThreshold();


    for(int i=0; i < children.size(); ++i)
    {
      insertMap(children[i], mymap);
      csv << mymap.find(children[i])->second; 
    }
    csv << endrow;
    for(int i=0; i < edges.size(); ++i)
    {
      printCSV(children[i], csv, treeLevel+1, mymap);
    }
  }
}

// Print function to print out tree content to reproduce the text from the input text file.

void printTree(treeNode* root, int treeLevel)
{
  if(root == nullptr)
    return;

  vector<treeEdge> edges = root->getEdges();
  vector<treeNode*> children = root->getChildren();
  // the edge and child node are loosely coupled now, check the numbers before printing.   
  if(edges.size() != children.size())
  {
    cerr<< "edges and children nodes number mismatch" << endl;
    return;
  }    

  if(root->isLeafNode())
  {
    cout << ": " << root->getLabel() ;
    cout << " (" << root->getObservedNum(); 
    if(root->getMisClassfied() > 0.f)
      cout << "/" <<  root->getMisClassfied();
    cout << ")";
  }
  else
  {
    for(int i=0; i < edges.size(); ++i)
    {
      if(root->isLeafNode() == false)
      {
        cout << "\n";
        for(int i=0; i < treeLevel; ++i)
          cout << "|   " ;
      }

      cout <<  root->getFeature() << "" << edges[i].getRelation() << " " << edges[i].getThreshold();
      printTree(children[i], treeLevel+1);
    }
  }
}

int main(int argc, char *argv[]){

   const char* inputfile = argv[1];
  
   // test and check regex at https://regex101.com/ but need to replace "\\" with "\"  
   std::regex re("\\s*([\\w\\s.-]+)\\s*(<=|>)\\s*([\\d\\.]+)\\s*(:\\s*(\\d+)\\s*\\(([\\d\\.]+)(\\/([\\d\\.]+))?\\))?");
   std::smatch match;
 
   fstream inputfilestream;
   inputfilestream.open(inputfile,ios::in); 


   treeNode* root = nullptr;

   if (inputfilestream.is_open()){ 
      std::stack<treeNode*> nodeStack;  // stack to trace the tree nodes, top of stack holds the latest parent node
      treeNode* cur;     
 
      string tp;
      while(getline(inputfilestream, tp)){ //read data from file object and put it into string.
         // when treeLevel increase, adding new edge and node, otherwise, adding only new edge
         size_t treeLevel = std::count(tp.begin(), tp.end(), '|');
         string content = tp.substr(tp.rfind("|") + 1); 

         //cout << "Original string:" << tp  << "\n";
         //cout << "Tree content:" << content << "\n";
         if (std::regex_search(content, match, re) == true) {
         
             // The size() member function indicates the
             // number of capturing groups plus one for the overall match
             // cout << "Match size = " << match.size() << endl;
        
             if(0) 
             {
               cout << "Whole match : " << match.str(0) << endl;
               for(int imatch = 0; imatch < match.size() ; ++imatch)
               { 
                 if(!match.str(imatch).empty())  
                   cout << "match " << imatch << " : " << match.str(imatch) << endl;
               }
             }

             string feature = match.str(1);
             string r = match.str(2);
             float threshold = std::stof(match.str(3));
             bool hasleaf = !match.str(4).empty();  // 4th matched group represents leaf node
             int label = (match.str(5).empty()) ? -999 : std::stoi(match.str(5));
             float numObserved = (match.str(6).empty()) ? 0.f : std::stoi(match.str(6));
             float numMisClassfied = (match.str(8).empty()) ? 0.f : std::stoi(match.str(8));
             //cout << "hasleaf:" << hasleaf << " feature: " << feature << "; relation: " << r << threshold  << " label: " << label << endl;
             relationtype rtype;
             if(r.compare("<=") == 0)
               rtype = relationtype::LE;
             else if(r.compare(">") == 0)
               rtype = relationtype::GT;
            
             treeEdge edge(rtype, threshold);
             //treeNode* newNode = new treeNode(feature, edge);
             //cout << "tree level:" << treeLevel << " stack size: " << nodeStack.size() << endl; 

             // adding root node and push to stack
             if (root == nullptr)
             {
               //cout << "adding root node" << endl;
               root = new treeNode(feature, edge);;
               cur = root;
             }
             else
             {
               if(treeLevel > nodeStack.size())
               {
                 //cout << "adding new node and edge:" << feature << endl;
                 // tree traversal going downward
                 treeNode* newNode = new treeNode(feature, edge);
                 cur->addChild(newNode);
                 //cout << "pushing:" << cur->getFeature() << endl;
                 nodeStack.push(cur);
                 cur = newNode;
               }
               else
               {
                 // tree traversal going upward, adding only new edge
                 while(treeLevel != nodeStack.size())
                 {
                   cur = nodeStack.top();
                   //cout << "popping:" << cur->getFeature() << endl;
                   nodeStack.pop();
                 }
                 cur->addEdge(edge);
               }
               if(hasleaf)
               {
                 //cout << "adding leaf from : " << cur->getFeature() << " sizeof stack: " << nodeStack.size()<< endl;
                 cur->addLeafNode(label, numObserved, numMisClassfied);
               } 
             }
         }

      }
      inputfilestream.close(); 
   }
   printTree(root, 0);


   try {
     csvfile csv("decisionTree.csv"); 

     csv << "NodeID" << "level" << "feature" << "label" << "hasChild"  << "relationOp" << "relationValue" << "trueNode" << "falseNode"  << endrow;

     unordered_map<treeNode*, int> mymap;

     printCSV(root, csv,  0, mymap);
   }
   catch (const std::exception &ex)
    {
        std::cout << "Exception was thrown: " << ex.what() << std::endl;
    }
  
    return 0;
}
