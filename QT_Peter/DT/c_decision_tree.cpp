#include "c_decision_tree.h"

#include <QFile>
#include <QTextStream>

c_decision_tree::c_decision_tree()
{

}

void c_decision_tree::LoadDataFile(QString filename, std::vector<QString> features)
{
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }
    mFeatures = features;
    QTextStream in(&file);
    QString line = in.readLine();
    while (!line.isNull())
    {
        QStringList attlist = line.split(",");
        if(attlist.size()-1!=features.size())return;
        DataPoint point ;
        for(uint i=0;i<features.size();i++)
        {
            //= &mDataSet[nPoint];
            point.mFeatureVals.insert(features[i],attlist.at(i).toDouble());
        }
        point.mTarget = attlist.at(attlist.size()-1).toInt()!=0;
        mDataSet.push_back(point);
        line = in.readLine();
    }
    printf("File loaded, num of features:%d, dataset size:%d\n",mFeatures.size(),mDataSet.size());
}

void c_decision_tree::run()
{
    isLeaf = false;
    mPrediction = 0;
    current_depth = 0;
    if(!split())create_leaf();
}
Node::Node(int depth, std::vector<DataPoint> dataSet, std::vector<QString> features) {
    mPrediction = 0;
    isLeaf = false;
    current_depth = depth;
    mDataSet = dataSet;
    mFeatures = features;
    if(!split())create_leaf();
}

void Node::create_leaf()
{
    isLeaf = true;
    //find majority
    int nTrue=0;
    for(uint j=0;j<mDataSet.size();j++)
    {
        if(mDataSet[j].mTarget)nTrue++;
    }
    mPrediction = (double)nTrue/double(mDataSet.size());
    printf("prediction:%f",mPrediction);
}
int Node::num_mistakes(DataSet data)//intermediate_node_num_mistakes
{
    int nTrue = 0;
    for(uint i=0;i<data.size();i++)
    {
        if(data[i].mTarget)nTrue++;
    }
    int nFalse = data.size()-nTrue;
    if(nFalse<nTrue)return nFalse*LOST_PRECISION;// mistakes are precision(false detections)
    else return nTrue*LOST_RECALL;// mistakes are recall(skipped true detections)
}

bool Node::split()//finds the best feature to split on given the data and a list of features
{
    if(current_depth>MAX_DEPTH)return false;//leaf condition
    if(mDataSet.size()<20)
    {
        printf("stop by mDataSet.size():%d, ",mDataSet.size());
        return false;//leaf condition
    }
    double     errorBefore = num_mistakes(mDataSet);
    if(errorBefore/double(mDataSet.size())>MAX_ACCURACY)return false;//leaf condition
    double  best_error  = errorBefore;
    int     n_split_point  = mDataSet.size()/150;
    for(uint i=0;i<mFeatures.size();i++)
    {
        std::vector<double> split_points;
        std::vector<double> values;
        for(uint j=0;j<mDataSet.size();j++)//get values
        {
            if(mDataSet[j].mFeatureVals.find(mFeatures[i])==mDataSet[j].mFeatureVals.end())continue;
            values.push_back(mDataSet[j].mFeatureVals.value(mFeatures[i]));
        }
        std::sort(values.begin(),values.end());
        int step = values.size()/(n_split_point+1);
        for(int j =0;j<n_split_point;j++)
        {
            split_points.push_back(values[step*(j+1)]);
        }
        int split_pos=-1;
        for(int j =0;j<n_split_point;j++)
        {
            double error = get_error_of_split(mFeatures[i],split_points[j]);
            if(error<best_error)
            {
                split_pos = (j+1)*step;
                best_error = error;
                best_feature = mFeatures[i];
                best_split_point = split_points[j];
            }
        }
        int iter = 1;
        while (split_pos>0)
        {
            step/=iter;iter++;
            int pos_right = split_pos+step;
            int pos_left = split_pos-step;
            if(pos_right>values.size()-1)break;
            if(pos_left<1)break;
            split_pos = -1;
            double errorLeft = get_error_of_split(mFeatures[i],values[pos_left]);
            double errorRight = get_error_of_split(mFeatures[i],values[pos_right]);
            if(errorLeft<best_error)
            {
                split_pos = pos_left;
                best_error = errorLeft;
                best_split_point = values[pos_left];
            }
            else if(errorRight<best_error)
            {
                split_pos = pos_right;
                best_error = errorRight;
                best_split_point = values[pos_right];
            }
        }
    }
    double errorGain = (errorBefore-best_error)/double(mDataSet.size());
    if(errorGain<MIN_ERROR_GAIN)
    {
        printf("stop by gain, ");
        return false;//leaf condition
    }
    DataSet  mDataLeft;
    DataSet  mDataRight;
    for(uint j=0;j<mDataSet.size();j++)
    {
        double value = mDataSet[j].mFeatureVals.value(best_feature);
        if(value<best_split_point)
            mDataLeft.push_back(mDataSet[j]);
        else
            mDataRight.push_back(mDataSet[j]);
    }
    printf("\nsplit by feature: ");
    printf(best_feature.toStdString().data());
    printf(" at value:%f",best_split_point);
    printf("\nLeft %d:",current_depth+1);
    mLeftNode = new Node(current_depth+1,mDataLeft,mFeatures);
    printf("\nRight %d:",current_depth+1);
    mRightNode = new Node(current_depth+1,mDataRight,mFeatures);
    return true;
}

bool Node::Predict(DataPoint point)
{
    if(isLeaf)return mPrediction;
    else
    {
        if(point.mFeatureVals.value(best_feature)<best_split_point)
            return mLeftNode->Predict(point);
        else
            return mRightNode->Predict(point);
    }
}
int Node::get_error_of_split(QString feature,double split_point)
{

    DataSet  mDataLeft;
    DataSet  mDataRight;
    for(int j=0;j<mDataSet.size();j++)
    {
        double value = mDataSet[j].mFeatureVals.value(feature);
        if(value<split_point)
            mDataLeft.push_back(mDataSet[j]);
        else
            mDataRight.push_back(mDataSet[j]);
    }
    return num_mistakes(mDataLeft)+num_mistakes(mDataRight);

}
