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
		BranchNode* parent = nullptr;                        //�����
		Envelope envelope;                                   //���İ�Χ��
		virtual int NodeType() = 0;                          //�������
		virtual int getHeight(int* h) = 0;                   //���ڼ������ĸ߶�
		virtual void traverse(vector<Envelope>& env) = 0;    //�����������н�㣬��������Χ�д���env��
		virtual void draw()=0;                               //���ƽ���Χ��
		virtual int MBRCount() = 0;                          //��Χ����Ŀ
		virtual vector<Envelope> subEnvelope() = 0;          //�ӽڵ��Ҫ�صİ�Χ��
		virtual LeafNode* PickLeaf(Feature* f)=0;            //Ѱ�����ĸ�Ҷ�ӽڵ����Ҫ��
		vector<int> PickSeed();                              //Ѱ�����ӵ�
		vector<vector<int>> SeedGroup(vector<int> seed);     //���ӽ����ݺ����ӵ��Զ������
		virtual RTNode* Split() = 0;                         //�ӽ���Ҫ�����˺���ѳ������µĽڵ�
		void AdjustEnvelope();                               //�ϲ���Χ��
		RTNode* AdjustTree();                                //����������㲻�÷��ѣ�
		RTNode* AdjustTree(RTNode* newNode, int MaxFill);    //�������������Ҫ���ѣ�
		RTNode* Insert(Feature* f, int MaxFill);             //����Ҫ�أ�MaxFill�ǽ������
		virtual void RangeQuery(Envelope& searchArea, vector<Feature>& result) = 0;    //�����ѯ���������result��
		virtual bool PointinLeafNode(double x, double y, LeafNode*& node) = 0;         //�жϵ��Ƿ���Ҷ�ڵ��Χ����
		virtual void PointnearLeafNode(double x, double y, LeafNode*& node) = 0;       //��Ҷ�ڵ��Χ������ĵ�
		virtual void elementCount(int& cnt) = 0;
		virtual void printTree(int& nodenum) = 0;
	};

	class LeafNode : public RTNode {
	public:
		vector<Feature*> elements;//Ҫ��
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
		vector<RTNode*>childs;//�ӽڵ�
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
		bool NNQuery(double x, double y, vector<Feature>& features);//NN��ѯ�����������features
		void All_area();
		void getHeight();
		bool RangeQuery(Envelope& searchArea, vector<Feature>& result);
		int elememtCount() { int cnt = 0; root->elementCount(cnt); return cnt; }
		void printTree() { int NodeNum = 0; root->printTree(NodeNum); }
	};

}


#endif

