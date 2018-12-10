#ifndef C_DECISION_TREE_H
#define C_DECISION_TREE_H

#include <QString>
#include <vector>
#include <QHash>
#define MAX_ACCURACY 0.99
#define MAX_DEPTH 10
#define MIN_ERROR_GAIN 0.01
#define LOST_PRECISION 1
#define LOST_RECALL 1
struct Feature
{
    QString mName;
    bool isContinuos;
    double valueCt;
    QString valueNonCt;
};
struct DataPoint
{
    QHash<QString,double> mFeatureVals;
    bool mTarget;
};

typedef std::vector<DataPoint> DataSet;
class Node
{
public:
    Node(){}
    Node(int depth,std::vector<DataPoint>  dataSet,std::vector<QString> features);
    Node *mLeftNode,*mRightNode;
    DataSet  mDataSet;
    std::vector<QString>    mFeatures;
    double mPrediction;
    bool isLeaf;
    bool reached_minimum_node_size();//Return true if the number of data points is less than or equal to the minimum node size.
    int num_mistakes(DataSet data);
    bool split();
    bool Predict(DataPoint point);
    void create_leaf();
    int current_depth;
private:

    QString best_feature;
    double  best_split_point;
    int get_error_of_split(QString feature, double split_point);
};
class c_decision_tree:public Node
{
public:
    c_decision_tree();
    void LoadDataFile(QString filename, std::vector<QString> features);
    void run();
private:


};

#endif // C_DECISION_TREE_H
