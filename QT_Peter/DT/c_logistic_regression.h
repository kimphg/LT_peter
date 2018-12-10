#ifndef C_LOGISTIC_REGRESSION_H
#define C_LOGISTIC_REGRESSION_H


class c_logistic_regression
{
public:
    c_logistic_regression();
    double predict_probability(feature_matrix, coefficients);
};

#endif // C_LOGISTIC_REGRESSION_H
