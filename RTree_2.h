#ifndef RTREE_2_H_INCLUDED
#define RTREE_2_H_INCLUDED

#include "Geometry.h"
#include "QuadTree.h"

namespace hw6 {

	class LeafNode;
	class BranchNode;
	class RTNode;

    class RTNode {
	public:
		BranchNode* parent = nullptr;                        //父结点
		Envelope envelope;                                   //结点的包围盒
		virtual int NodeType() = 0;                          //结点类型
		virtual int getHeight(int* h) = 0;                   //用于计算树的高度
		virtual void traverse(vector<Envelope>& env) = 0;    //遍历书中所有结点，将各结点包围盒存入env中
		virtual void draw()=0;                               //绘制结点包围盒
		virtual int MBRCount() = 0;                          //包围盒数目
		virtual vector<Envelope> subEnvelope() = 0;          //子节点或要素的包围盒
		virtual LeafNode* PickLeaf(Feature* f)=0;            //寻找在哪个叶子节点插入要素
		vector<int> PickSeed();                              //寻找种子点
		vector<vector<int>> SeedGroup(vector<int> seed);     //将子结点跟据和种子点的远近分组
		virtual RTNode* Split() = 0;                         //子结点或要素满了后分裂成两个新的节点
		void AdjustEnvelope();                               //合并包围盒
		RTNode* AdjustTree();                                //调整树（结点不用分裂）
		RTNode* AdjustTree(RTNode* newNode, int MaxFill);    //调整树（结点需要分裂）
		RTNode* Insert(Feature* f, int MaxFill);             //插入要素，MaxFill是结点容量
		virtual void RangeQuery(Envelope& searchArea, vector<Feature>& result) = 0;    //区域查询，结果存入result中
		virtual bool PointinLeafNode(double x, double y, LeafNode*& node) = 0;         //判断点是否在叶节点包围盒中
		virtual void PointnearLeafNode(double x, double y, LeafNode*& node) = 0;       //离叶节点包围盒最近的点
		virtual void elementCount(int& cnt) = 0;
		virtual void printTree(int& nodenum) = 0;
	};

	class LeafNode : public RTNode {
	public:
		vector<Feature*> elements;//要素
		int MBRCount();
		int NodeType()override { return 1; }
		vector<Envelope> subEnvelope()override;
		void RangeQuery(Envelope& searchArea, vector<Feature>& result)override;
		bool PointinLeafNode(double x, double y, LeafNode*& node);
		void PointnearLeafNode(double x, double y, LeafNode*& node);
		LeafNode* PickLeaf(Feature* f)override;
		RTNode* Split()override;
		void draw()override;
		void traverse(vector<Envelope>& env)override;
		int getHeight(int* h)override;
		void elementCount(int& cnt)override;
		void printTree(int& nodenum)override;
	};

	class BranchNode : public RTNode {
	public:
		vector<RTNode*>childs;//子节点
		int MBRCount();
		int NodeType()override { return 2; }
		vector<Envelope> subEnvelope()override;
		void RangeQuery(Envelope& searchArea, vector<Feature>& result)override;
		bool PointinLeafNode(double x, double y, LeafNode*& node);
		void PointnearLeafNode(double x, double y, LeafNode*& node);
		LeafNode* PickLeaf(Feature* f)override;
		RTNode* Split()override;
		void draw()override;
		void traverse(vector<Envelope>& env)override;
		int getHeight(int* h)override;
		void elementCount(int& cnt)override;
		void printTree(int& nodenum)override;
	};
	class RTree {
	public:
		RTNode* root=nullptr;
		Envelope MBR;
		int MaxFill;
		void draw() { if (root)root->draw(); }
		RTree() { this->root = new LeafNode(); MaxFill = 0; }
		RTNode* Construction(vector<Feature*>f, int MaxFill);
		bool NNQuery(double x, double y, vector<Feature>& features);//NN查询，将结果存入features
		void All_area();
		void getHeight();
		bool RangeQuery(Envelope& searchArea, vector<Feature>& result);
		int elememtCount() { int cnt = 0; root->elementCount(cnt); return cnt; }
		void printTree() { int NodeNum = 0; root->printTree(NodeNum); }
	};

}


#endif

