#include "RTree_2.h"

namespace hw6 {
    
	//MBRCount:返回当前结点内的包围盒数
	int LeafNode::MBRCount() { return this->elements.size(); }
	int BranchNode::MBRCount() { return this->childs.size(); }

	//返回当前节点下的所有包围盒
	vector<Envelope> LeafNode::subEnvelope()
	{
		vector<Envelope> subs;
		for (Feature* elm : elements)
		{
			Envelope el = elm->getEnvelope();
			subs.push_back(el);
		}
		return subs;
	}
	vector<Envelope> BranchNode::subEnvelope()
	{
		vector<Envelope> subs;
		for (RTNode* Child : childs) {
			subs.push_back(Child->envelope);
		}
		return subs;
	}

	//找到插入结点时面积扩张最小的叶节点
	LeafNode* BranchNode::PickLeaf(Feature* f)
	{
		double minDiff = DBL_MAX;//最小的扩张面积
		Envelope env = f->getEnvelope();//获取要插入要素的包围盒
		double curArea, diff;
		Envelope Cover(DBL_MAX, -DBL_MAX, DBL_MAX, -DBL_MAX);//合并后的包围盒
		RTNode* minChild;//插入要素后包围盒扩张面积最小的结点
		for (RTNode* Child : childs) {
			curArea = Child->envelope.getArea();//子结点包围盒面积
			Cover = Child->envelope.unionEnvelope(env);//合并
			diff = Cover.getArea() - curArea;//计算扩张面积
			if (diff <minDiff) {//判断扩张面积是否是最小
				minDiff = diff; minChild = Child;
			}
			else if (diff == minDiff) {//扩张面积相同选择包围盒数量较小的
				if (Child->MBRCount() < minChild->MBRCount()) {
					minChild = Child;
				}
			}
		}
		//递归到叶子节点，找到扩张最小的结点
		return minChild->PickLeaf(f);
	}
	LeafNode* LeafNode::PickLeaf(Feature* f)
	{
		return this;//递归找到叶结点后返回选中的叶结点
	}


	//找种子点，需要相隔尽可能远,选择合并前后包围盒面积相差最大的两个结点
	vector<int> RTNode::PickSeed()
	{
		vector<int>Seed(2);//要挑选两个种子点
		vector<Envelope> &subs = this->subEnvelope(); //当前结点子结点的包围盒

		double biggestDiff = -1;//初始化最大面积差
		for (int i = 0; i < subs.size() - 1; i++) {//逐个要素比较
			for (int j = 1; j < subs.size(); j++) {
				Envelope Cover = subs[i].unionEnvelope(subs[j]);//合并后的MBR
				double CoverArea = Cover.getArea();//合并后MBR面积
				double diff = CoverArea - subs[j].getArea();//合并后的面积增长值
				if (diff > biggestDiff) {
					biggestDiff = diff;
					Seed[0] = i;
					Seed[1] = j;
				}
			}
		}
		return Seed;//返回两个种子点的编号
	}

	//将节点的编号按与种子点的面积差分配到两个组中
	vector<vector<int>> RTNode::SeedGroup(vector<int> seed)
	{
		vector<vector<int>> Group(2);//分成两个组，每个组存放包围盒的id
		Group[0].push_back(seed[0]);//添加种子结点
		Group[1].push_back(seed[1]);

		vector<Envelope> &subs = this->subEnvelope();
		for (int i = 0; i < subs.size(); i++) {//遍历节点下的包围盒
			if (i == seed[0] || i == seed[1])continue;//种子点跳过
			//不是种子点，计算面积增量，添加到小的一组
			Envelope Cover0 = subs[i].unionEnvelope(subs[seed[0]]);
			double diff0 = Cover0.getArea() - subs[seed[0]].getArea();
			Envelope Cover1 = subs[i].unionEnvelope(subs[seed[1]]);
			double diff1 = Cover1.getArea() - subs[seed[1]].getArea();

			if (diff0 > diff1)Group[1].push_back(i);//选择面积增长较小的加入
			else Group[0].push_back(i);
		}
		return Group;
	}


	//分裂叶节点
	RTNode* LeafNode::Split()
	{
		//创建新节点，并根据分组分配要素
		vector<vector<int>> group = this->SeedGroup(this->PickSeed());//两个组，每个组内存的是要素的编号
		vector<Feature*> temp = this->elements;
		this->elements.clear();//清除原先节点下的要素
		LeafNode* newNode = new LeafNode();//创建新的叶结点
		Envelope e(DBL_MAX, -DBL_MAX, DBL_MAX, -DBL_MAX);
		newNode->envelope = e;
		newNode->parent = this->parent;//新结点和旧结点指向同一个父结点
		//分配要素
		for (int i = 0; i < group[0].size(); i++)
			this->elements.push_back(temp[group[0][i]]);
		for (int j = 0; j < group[1].size(); j++)
			newNode->elements.push_back(temp[group[1][j]]);

		//调整包围盒，使当前结点包围盒包围所有要素
		this->AdjustEnvelope();
		newNode->AdjustEnvelope();
		return newNode;//返回新结点
	}

	//分裂分支结点，在AdjustTree中使用
	RTNode* BranchNode::Split()
	{
		vector<vector<int>> group = this->SeedGroup(this->PickSeed());
		vector<RTNode*>temp = this->childs;
		this->childs.clear();
		BranchNode* newNode = new BranchNode();//创建新的分支结点
		Envelope e(DBL_MAX, -DBL_MAX, DBL_MAX, -DBL_MAX);
		newNode->envelope = e;
		newNode->parent = this->parent;
		for (int i = 0; i < group[0].size(); i++)
			this->childs.push_back(temp[group[0][i]]);
		for (int j = 0; j < group[1].size(); j++){
			newNode->childs.push_back(temp[group[1][j]]);
			temp[group[1][j]]->parent = newNode;//让新节点分配到的子结点都指向它
		}	
		return newNode;
	}


	//合并包围盒
	void RTNode::AdjustEnvelope()
	{
		Envelope curEnv(DBL_MAX,-DBL_MAX, DBL_MAX, -DBL_MAX);
		vector<Envelope>& subs = this->subEnvelope();
		for (Envelope sub : subs)
			curEnv = curEnv.unionEnvelope(sub);
		this->envelope = curEnv;
	}

	//从底部开始向上调整R树，最终返回根节点
	RTNode* RTNode::AdjustTree() {//无参，结点不分裂
		this->AdjustEnvelope();//合并当前结点里保存的元素的包围盒
		if (parent == nullptr)return this;//递归出口，即到了根节点
		return parent->AdjustTree();//向上递归
	}

	RTNode* RTNode::AdjustTree(RTNode* newNode,int MaxFill)
	{//有参，结点需要分裂
		if (parent == nullptr) {//到了根节点还要继续分裂，需要生成新的根节点
			BranchNode* newRoot = new BranchNode();//根节点为分支结点
			this->parent = newRoot;//旧结点指向新的父节点
			newNode->parent = newRoot;//新结点指向新父节点
			newRoot->childs.push_back(this);
			newRoot->childs.push_back(newNode);
			newRoot->AdjustEnvelope();
			return newRoot;//返回新的根结点
		}
		//没到根节点
		parent->childs.push_back(newNode);
		//因为分裂后父节点会多出一个子元素，所以仍有可能溢出，需要再进行一次判断
		//如果父节点不会溢出，则不用继续分裂
		if (parent->MBRCount() <= MaxFill) {
			parent->AdjustEnvelope();
			return parent->AdjustTree();
		}
		else {//父节点溢出，需要继续分裂
			RTNode* oldP = this->parent;
			RTNode* newP = parent->Split();
			return oldP->AdjustTree(newP,MaxFill);
		}
	}

	//插入要素，返回树根节点
	RTNode* RTNode::Insert(Feature* f,int MaxFill)
	{
		//选择在哪里进行插入
		LeafNode* leaf2insert = this->PickLeaf(f);
		leaf2insert->elements.push_back(f);

		//判断节点是否溢出，是否需要分裂
		if (leaf2insert->MBRCount() > MaxFill)
		{
			//创建新的叶节点
			LeafNode* newLeaf = (LeafNode*)leaf2insert->Split();
			return leaf2insert->AdjustTree(newLeaf,MaxFill);
		}
		else {
			//不需要分裂
			return leaf2insert->AdjustTree();
		}
	}

	void BranchNode::RangeQuery(Envelope& searchArea,
		vector<Feature>& result) {
		if (!envelope.intersect(searchArea)) {
			return;
		}
		for (RTNode* child : childs) {
			if(child->envelope.intersect(searchArea))
			child->RangeQuery(searchArea, result);//递归查找下一层
		}

	}
	void LeafNode::RangeQuery(Envelope& searchArea,
		vector<Feature>& result) {
		for (Feature* v : elements) {
			if ((v->getEnvelope()).intersect(searchArea)) {
				result.push_back(*v);//把叶节点中有重合的元素加入result中
			}
		}
		return;
	}

	bool BranchNode::PointinLeafNode(double x, double y, LeafNode*& node) {
		if (!envelope.contain(x, y)) {
			return false;
		}
		for (RTNode* child : childs) {
			return child->PointinLeafNode(x, y, node);//递归查找下一层
		}
	}
	bool LeafNode::PointinLeafNode(double x, double y, LeafNode*& node) {
		if (!envelope.contain(x, y)) {
			return false;
		}
		for (Feature* f : elements) {
			if (f->getEnvelope().contain(x, y)) {
				node = this;//找到了，赋给node
				return true;
			}
		}
		return false;
	}

	bool RTree::NNQuery(double x, double y, vector<Feature>& features) {
		if (!root)return false;
		double minDist = max(root->envelope.getWidth(), root->envelope.getHeight());
		LeafNode* n = new LeafNode();
		root->PointnearLeafNode(x, y, n);
		for (Feature* f : n->elements) {
			minDist = min(minDist, f->maxDistance2Envelope(x, y));
		}
		Envelope rect = Envelope(x - minDist, x + minDist, y - minDist, y + minDist);
		root->RangeQuery(rect, features);//features1为包围盒与rect区域相交的几何特征集合
		return true;
	}

void BranchNode::PointnearLeafNode(double x, double y, LeafNode*& node) {
	double mindist = INFINITY;
	int minid = -1;
	for (int i = 0;i < childs.size();i++) {
		if (childs[i]->envelope.distance(x, y) <= mindist) {
			mindist = childs[i]->envelope.distance(x, y);
			minid = i;
		}
	}
	if(minid!=-1)
		childs[minid]->PointnearLeafNode(x, y, node);
}
void LeafNode::PointnearLeafNode(double x, double y, LeafNode*& node) {
	node = this;
}

	void LeafNode::draw()
	{
		this->envelope.draw();
		for (Feature* f : elements) {
			f->getEnvelope().draw();
		}
	}
	void BranchNode::draw()
	{
		this->envelope.draw();
		for (RTNode* e : childs) {
			e->draw();
		}
	}
	void LeafNode::traverse(vector<Envelope>& env)
	{
		env.push_back(this->envelope);
		for (int i = 0; i < this->subEnvelope().size(); i++) {
			vector<Envelope> e = this->subEnvelope();
			env.push_back(e[i]);
		}
	}
	void BranchNode::traverse(vector<Envelope>& env)
	{
		env.push_back(this->envelope);
		for (RTNode* child : childs) {
			child->traverse(env);
		}
	}

	int LeafNode::getHeight(int* h)
	{
		return *h;
	}
	int BranchNode::getHeight(int* h)
	{
		(*h)++;
		RTNode* Child = childs[0];
		Child->getHeight(h);
		return *h;
	}

	void BranchNode::elementCount(int& cnt)
	{
		for (int i = 0; i < childs.size(); i++) {
			childs[i]->elementCount(cnt);
		}

	}
	void LeafNode::elementCount(int& cnt)
	{
		cnt += this->elements.size();
		return;
	}

	void BranchNode::printTree(int& nodenum)
	{
		for (int i = 0; i < childs.size(); i++)
			childs[i]->printTree(nodenum);
	}
	void LeafNode::printTree(int& nodenum)
	{
		cout << "Node[" << nodenum++ << "] : " << endl;
		for (int i = 0; i < elements.size(); i++) {
			cout << "feature[" << i << "] : x=" << elements[i]->getEnvelope().getMinX() << "  y=" << elements[i]->getEnvelope().getMinY() << endl;
		}
	}

	
	RTNode* RTree::Construction(vector<Feature*>f, int MaxFill) {
		root = new LeafNode();
		Envelope e(DBL_MAX, -DBL_MAX, DBL_MAX, -DBL_MAX);
		root->envelope = e;
		this->MBR = e;
		this->MaxFill = MaxFill;
		for (int i = 0; i < f.size(); i++) {
			root = root->Insert(f[i], MaxFill);
		}
		MBR = MBR.unionEnvelope(root->envelope);

		return this->root;
	}

	void RTree::All_area() {
		vector<Envelope> s;
		this->root->traverse(s);
		for (Envelope& e : s) {
			cout << e.getMinX() << " " << e.getMinY() << " " << e.getMaxX() << " " << e.getMaxY() << endl;
		}
	}

	void RTree::getHeight() {
		int height = 0;
		this->root->getHeight(&height);
		cout << height+1 << endl;
	}

	bool RTree::RangeQuery(Envelope& searchArea, vector<Feature>& result) {
		result.clear();
		if (searchArea.intersect(root->envelope))
			root->RangeQuery(searchArea, result);
		if (!result.empty())return true;
		else return false;
	}
}