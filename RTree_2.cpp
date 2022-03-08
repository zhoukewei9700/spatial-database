#include "RTree_2.h"

namespace hw6 {
    
	//MBRCount:���ص�ǰ����ڵİ�Χ����
	int LeafNode::MBRCount() { return this->elements.size(); }
	int BranchNode::MBRCount() { return this->childs.size(); }

	//���ص�ǰ�ڵ��µ����а�Χ��
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

	//�ҵ�������ʱ���������С��Ҷ�ڵ�
	LeafNode* BranchNode::PickLeaf(Feature* f)
	{
		double minDiff = DBL_MAX;//��С���������
		Envelope env = f->getEnvelope();//��ȡҪ����Ҫ�صİ�Χ��
		double curArea, diff;
		Envelope Cover(DBL_MAX, -DBL_MAX, DBL_MAX, -DBL_MAX);//�ϲ���İ�Χ��
		RTNode* minChild;//����Ҫ�غ��Χ�����������С�Ľ��
		for (RTNode* Child : childs) {
			curArea = Child->envelope.getArea();//�ӽ���Χ�����
			Cover = Child->envelope.unionEnvelope(env);//�ϲ�
			diff = Cover.getArea() - curArea;//�����������
			if (diff <minDiff) {//�ж���������Ƿ�����С
				minDiff = diff; minChild = Child;
			}
			else if (diff == minDiff) {//���������ͬѡ���Χ��������С��
				if (Child->MBRCount() < minChild->MBRCount()) {
					minChild = Child;
				}
			}
		}
		//�ݹ鵽Ҷ�ӽڵ㣬�ҵ�������С�Ľ��
		return minChild->PickLeaf(f);
	}
	LeafNode* LeafNode::PickLeaf(Feature* f)
	{
		return this;//�ݹ��ҵ�Ҷ���󷵻�ѡ�е�Ҷ���
	}


	//�����ӵ㣬��Ҫ���������Զ,ѡ��ϲ�ǰ���Χ�������������������
	vector<int> RTNode::PickSeed()
	{
		vector<int>Seed(2);//Ҫ��ѡ�������ӵ�
		vector<Envelope> &subs = this->subEnvelope(); //��ǰ����ӽ��İ�Χ��

		double biggestDiff = -1;//��ʼ����������
		for (int i = 0; i < subs.size() - 1; i++) {//���Ҫ�رȽ�
			for (int j = 1; j < subs.size(); j++) {
				Envelope Cover = subs[i].unionEnvelope(subs[j]);//�ϲ����MBR
				double CoverArea = Cover.getArea();//�ϲ���MBR���
				double diff = CoverArea - subs[j].getArea();//�ϲ�����������ֵ
				if (diff > biggestDiff) {
					biggestDiff = diff;
					Seed[0] = i;
					Seed[1] = j;
				}
			}
		}
		return Seed;//�����������ӵ�ı��
	}

	//���ڵ�ı�Ű������ӵ���������䵽��������
	vector<vector<int>> RTNode::SeedGroup(vector<int> seed)
	{
		vector<vector<int>> Group(2);//�ֳ������飬ÿ�����Ű�Χ�е�id
		Group[0].push_back(seed[0]);//������ӽ��
		Group[1].push_back(seed[1]);

		vector<Envelope> &subs = this->subEnvelope();
		for (int i = 0; i < subs.size(); i++) {//�����ڵ��µİ�Χ��
			if (i == seed[0] || i == seed[1])continue;//���ӵ�����
			//�������ӵ㣬���������������ӵ�С��һ��
			Envelope Cover0 = subs[i].unionEnvelope(subs[seed[0]]);
			double diff0 = Cover0.getArea() - subs[seed[0]].getArea();
			Envelope Cover1 = subs[i].unionEnvelope(subs[seed[1]]);
			double diff1 = Cover1.getArea() - subs[seed[1]].getArea();

			if (diff0 > diff1)Group[1].push_back(i);//ѡ�����������С�ļ���
			else Group[0].push_back(i);
		}
		return Group;
	}


	//����Ҷ�ڵ�
	RTNode* LeafNode::Split()
	{
		//�����½ڵ㣬�����ݷ������Ҫ��
		vector<vector<int>> group = this->SeedGroup(this->PickSeed());//�����飬ÿ�����ڴ����Ҫ�صı��
		vector<Feature*> temp = this->elements;
		this->elements.clear();//���ԭ�Ƚڵ��µ�Ҫ��
		LeafNode* newNode = new LeafNode();//�����µ�Ҷ���
		Envelope e(DBL_MAX, -DBL_MAX, DBL_MAX, -DBL_MAX);
		newNode->envelope = e;
		newNode->parent = this->parent;//�½��;ɽ��ָ��ͬһ�������
		//����Ҫ��
		for (int i = 0; i < group[0].size(); i++)
			this->elements.push_back(temp[group[0][i]]);
		for (int j = 0; j < group[1].size(); j++)
			newNode->elements.push_back(temp[group[1][j]]);

		//������Χ�У�ʹ��ǰ����Χ�а�Χ����Ҫ��
		this->AdjustEnvelope();
		newNode->AdjustEnvelope();
		return newNode;//�����½��
	}

	//���ѷ�֧��㣬��AdjustTree��ʹ��
	RTNode* BranchNode::Split()
	{
		vector<vector<int>> group = this->SeedGroup(this->PickSeed());
		vector<RTNode*>temp = this->childs;
		this->childs.clear();
		BranchNode* newNode = new BranchNode();//�����µķ�֧���
		Envelope e(DBL_MAX, -DBL_MAX, DBL_MAX, -DBL_MAX);
		newNode->envelope = e;
		newNode->parent = this->parent;
		for (int i = 0; i < group[0].size(); i++)
			this->childs.push_back(temp[group[0][i]]);
		for (int j = 0; j < group[1].size(); j++){
			newNode->childs.push_back(temp[group[1][j]]);
			temp[group[1][j]]->parent = newNode;//���½ڵ���䵽���ӽ�㶼ָ����
		}	
		return newNode;
	}


	//�ϲ���Χ��
	void RTNode::AdjustEnvelope()
	{
		Envelope curEnv(DBL_MAX,-DBL_MAX, DBL_MAX, -DBL_MAX);
		vector<Envelope>& subs = this->subEnvelope();
		for (Envelope sub : subs)
			curEnv = curEnv.unionEnvelope(sub);
		this->envelope = curEnv;
	}

	//�ӵײ���ʼ���ϵ���R�������շ��ظ��ڵ�
	RTNode* RTNode::AdjustTree() {//�޲Σ���㲻����
		this->AdjustEnvelope();//�ϲ���ǰ����ﱣ���Ԫ�صİ�Χ��
		if (parent == nullptr)return this;//�ݹ���ڣ������˸��ڵ�
		return parent->AdjustTree();//���ϵݹ�
	}

	RTNode* RTNode::AdjustTree(RTNode* newNode,int MaxFill)
	{//�вΣ������Ҫ����
		if (parent == nullptr) {//���˸��ڵ㻹Ҫ�������ѣ���Ҫ�����µĸ��ڵ�
			BranchNode* newRoot = new BranchNode();//���ڵ�Ϊ��֧���
			this->parent = newRoot;//�ɽ��ָ���µĸ��ڵ�
			newNode->parent = newRoot;//�½��ָ���¸��ڵ�
			newRoot->childs.push_back(this);
			newRoot->childs.push_back(newNode);
			newRoot->AdjustEnvelope();
			return newRoot;//�����µĸ����
		}
		//û�����ڵ�
		parent->childs.push_back(newNode);
		//��Ϊ���Ѻ󸸽ڵ����һ����Ԫ�أ��������п����������Ҫ�ٽ���һ���ж�
		//������ڵ㲻����������ü�������
		if (parent->MBRCount() <= MaxFill) {
			parent->AdjustEnvelope();
			return parent->AdjustTree();
		}
		else {//���ڵ��������Ҫ��������
			RTNode* oldP = this->parent;
			RTNode* newP = parent->Split();
			return oldP->AdjustTree(newP,MaxFill);
		}
	}

	//����Ҫ�أ����������ڵ�
	RTNode* RTNode::Insert(Feature* f,int MaxFill)
	{
		//ѡ����������в���
		LeafNode* leaf2insert = this->PickLeaf(f);
		leaf2insert->elements.push_back(f);

		//�жϽڵ��Ƿ�������Ƿ���Ҫ����
		if (leaf2insert->MBRCount() > MaxFill)
		{
			//�����µ�Ҷ�ڵ�
			LeafNode* newLeaf = (LeafNode*)leaf2insert->Split();
			return leaf2insert->AdjustTree(newLeaf,MaxFill);
		}
		else {
			//����Ҫ����
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
			child->RangeQuery(searchArea, result);//�ݹ������һ��
		}

	}
	void LeafNode::RangeQuery(Envelope& searchArea,
		vector<Feature>& result) {
		for (Feature* v : elements) {
			if ((v->getEnvelope()).intersect(searchArea)) {
				result.push_back(*v);//��Ҷ�ڵ������غϵ�Ԫ�ؼ���result��
			}
		}
		return;
	}

	bool BranchNode::PointinLeafNode(double x, double y, LeafNode*& node) {
		if (!envelope.contain(x, y)) {
			return false;
		}
		for (RTNode* child : childs) {
			return child->PointinLeafNode(x, y, node);//�ݹ������һ��
		}
	}
	bool LeafNode::PointinLeafNode(double x, double y, LeafNode*& node) {
		if (!envelope.contain(x, y)) {
			return false;
		}
		for (Feature* f : elements) {
			if (f->getEnvelope().contain(x, y)) {
				node = this;//�ҵ��ˣ�����node
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
		root->RangeQuery(rect, features);//features1Ϊ��Χ����rect�����ཻ�ļ�����������
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