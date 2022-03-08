#include <set>
#include "QuadTree.h"

namespace hw6 {

	/*
	 * QuadNode
	 */
	void QuadNode::split(size_t capacity)
	{
		for (int i = 0; i < 4; ++i) {
			delete nodes[i];
			nodes[i] = NULL;
		}
		//clock_t start_time = clock();
		// Task construction
		// Write your code here
		//if (features.size() > capacity)
		//{
			/*double xMin = bbox.getMinX(); double xMax = bbox.getMaxX();
			double yMin = bbox.getMinY(); double yMax = bbox.getMaxY();
			double xMid = (xMax + xMin) / 2.0;
			double yMid = (yMax + yMin) / 2.0;

			//子包围盒
			Envelope env0(xMin, xMid, yMid, yMax); nodes[0] = new QuadNode(env0);
			Envelope env1(xMid, xMax, yMid, yMax); nodes[1] = new QuadNode(env1);
			Envelope env2(xMin, xMid, yMin, yMid); nodes[2] = new QuadNode(env2);
			Envelope env3(xMid, xMax, yMin, yMid); nodes[3] = new QuadNode(env3);*/
		
		Envelope e[4];
		double midX = (bbox.getMinX() + bbox.getMaxX()) / 2;
		double midY = (bbox.getMinY() + bbox.getMaxY()) / 2;
		e[0] = Envelope(bbox.getMinX(), midX, bbox.getMinY(), midY);
		e[1] = Envelope(midX, bbox.getMaxX(), bbox.getMinY(), midY);
		e[2] = Envelope(bbox.getMinX(), midX, midY, bbox.getMaxY());
		e[3] = Envelope(midX, bbox.getMaxX(), midY, bbox.getMaxY());

		nodes[0] = new QuadNode(e[0]);
		nodes[1] = new QuadNode(e[1]);
		nodes[2] = new QuadNode(e[2]);
		nodes[3] = new QuadNode(e[3]);
		for (int i = 0; i < features.size(); i++)
		{
			if (nodes[0]->getEnvelope().intersect(features[i].getEnvelope()))//判断要素包围盒和子包围盒是否相交
			{
				nodes[0]->add(features[i]);//相交则将要素加入子结点中
			}
			if (nodes[1]->getEnvelope().intersect(features[i].getEnvelope()))//判断要素包围盒和子包围盒是否相交
			{
				nodes[1]->add(features[i]);//相交则将要素加入子结点中
			}
			if (nodes[2]->getEnvelope().intersect(features[i].getEnvelope()))//判断要素包围盒和子包围盒是否相交
			{
				nodes[2]->add(features[i]);//相交则将要素加入子结点中
			}
			if (nodes[3]->getEnvelope().intersect(features[i].getEnvelope()))//判断要素包围盒和子包围盒是否相交
			{
				nodes[3]->add(features[i]);//相交则将要素加入子结点中
			}
		}
		features.clear();//清除当前节点的集合要素记录
		//clock_t end_time = clock();

		//cout << "split time: " << (end_time - start_time) / 1000.0 << "s" << endl;
		for (int i = 0; i < 4; i++)
		{
			if(nodes[i]->getFeatureNum()>capacity)
				nodes[i]->split(capacity);//递归分裂
		}
	}

	void QuadNode::countNode(int& interiorNum, int& leafNum)
	{
		if (isLeafNode()) {
			++leafNum;
		}
		else {
			++interiorNum;
			for (int i = 0; i < 4; ++i)
				nodes[i]->countNode(interiorNum, leafNum);
		}
	}

	int QuadNode::countHeight(int height)
	{
		++height;
		if (!isLeafNode()) {
			int cur = height;
			for (int i = 0; i < 4; ++i) {
				height = max(height, nodes[i]->countHeight(cur));
			}
		}
		return height;
	}

	void QuadNode::rangeQuery(Envelope& rect, vector<Feature>& features)
	{
		if (!bbox.intersect(rect))
			return;

		// Task range query
		// Write your code here
		/*if (isLeafNode()) {
			for (auto f : this->features) {
				if (f.getEnvelope().intersect(rect)) {
					features.push_back(f);
				}
			}
		}
		else {
			for (int i = 0; i < 4; ++i) {
				nodes[i]->rangeQuery(rect, features);
			}
		}*/
		for (int i = 0; i < 4; i++) {
			Envelope e = nodes[i]->bbox;
			if (bbox.intersect(nodes[i]->bbox)) {
				if (nodes[i]->isLeafNode()) {
					//store feature								
					for (int j = 0; j < nodes[i]->getFeatureNum(); j++) {
						Feature f = nodes[i]->getFeature(j);
						features.push_back(f);
					}

				}
				else {
					nodes[i]->rangeQuery(rect, features);
				}

			}
		}
	}

	QuadNode* QuadNode::pointInLeafNode(double x, double y)
	{
		// Task NN query
		// Write your code here
		if (isLeafNode()) {
			return this;
		}
		else {
			for (int i = 0; i < 4; i++) {
				if (nodes[i]->bbox.contain(x, y)) {
					return nodes[i]->pointInLeafNode(x, y);
				}
			}
		}
		
		return NULL;
	}

	void QuadNode::draw()
	{
		if (isLeafNode()) {
			bbox.draw();
		}
		else {
			for (int i = 0; i < 4; ++i)
				nodes[i]->draw();
		}
	}

	/*
	 * QuadTree
	 */
	bool QuadTree::constructQuadTree(vector<Feature>& features)
	{
		if (features.empty())
			return false;

		// Task construction
		// Write your code here
		Envelope ori_env(features[0].getEnvelope());//相当于空包围盒
		for (int i = 0; i < features.size(); i++)
		{
			ori_env = ori_env.unionEnvelope(features[i].getEnvelope());//合并所有要素的包围盒
		}
		root = new QuadNode(ori_env);
		root->add(features);
		if(root->getFeatureNum()>capacity)
		    root->split(capacity);

		bbox = ori_env;
		//bbox = Envelope(-74.1, -73.8, 40.6, 40.8); // 注意此行代码需要更新为features的包围盒，或根节点的包围盒

		return true;
	}

	void QuadTree::countQuadNode(int& interiorNum, int& leafNum)
	{
		interiorNum = 0;
		leafNum = 0;
		if (root)
			root->countNode(interiorNum, leafNum);
	}

	void QuadTree::countHeight(int& height)
	{
		height = 0;
		if (root)
			height = root->countHeight(0);
	}

	void QuadTree::rangeQuery(Envelope& rect, vector<Feature>& features)
	{
		features.clear();

		// Task range query
		// Write your code here
		if (rect.intersect(bbox))root->rangeQuery(rect, features);
		// 注意四叉树区域查询仅返回候选集，精炼步在hw6的rangeQuery中完成
	}

	bool QuadTree::NNQuery(double x, double y, vector<Feature>& features)
	{
		if (!root || !(root->getEnvelope().contain(x, y)))
			return false;

		// Task NN query
		// Write your code here

		// filter step (使用maxDistance2Envelope函数，获得查询点到几何对象包围盒的最短的最大距离，然后区域查询获得候选集)
		const Envelope& envelope = root->getEnvelope();
		double minDist = max(envelope.getWidth(), envelope.getHeight());

		QuadNode* n = root->pointInLeafNode(x, y);

		for (int i = 0; i < n->getFeatureNum(); i++) {
			minDist = min(minDist, n->getFeature(i).maxDistance2Envelope(x, y));
		}
		Envelope rect = Envelope(x - minDist, x + minDist, y - minDist, y + minDist);
		rangeQuery(rect, features);
		//const Envelope& envelope = root->getEnvelope();
		//double minDist = max(envelope.getWidth(), envelope.getHeight());
		// 注意四叉树邻近查询仅返回候选集，精炼步在hw6的NNQuery中完成

		return true;
	}

	void QuadTree::draw()
	{
		if (root)
			root->draw();
	}


}
