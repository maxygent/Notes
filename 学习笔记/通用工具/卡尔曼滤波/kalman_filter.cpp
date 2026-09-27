#include <iostream>
#include <iomanip>

struct KalmanFilter
{
    // 状态：
    // x = [position, velocity]^T
    double x[2] = {0.0, 0.0};

    // 状态协方差 P
    double P[2][2] = {
        {1.0, 0.0},
        {0.0, 1.0}
    };

    // 状态转移矩阵 F
    double F[2][2];

    // 观测矩阵 H
    // GPS只能测到 position
    double H[2] = {1.0, 0.0};

    // 过程噪声协方差 Q
    double Q[2][2];

    // 观测噪声协方差 R
    double R;

    double dt;

    KalmanFilter(double dt)
        : dt(dt)
    {
        // 状态转移：
        //
        // position = position + velocity * dt
        // velocity = velocity
        //
        // [p]     [1 dt] [p]
        // [v]  =  [0  1] [v]

        F[0][0] = 1.0;
        F[0][1] = dt;
        F[1][0] = 0.0;
        F[1][1] = 1.0;

        // 假设运动模型存在一定的不确定性
        Q[0][0] = 0.01;
        Q[0][1] = 0.0;
        Q[1][0] = 0.0;
        Q[1][1] = 0.01;

        // GPS测量噪声
        R = 1.0;
    }

    // Prediction
    void predict()
    {
        // x = F * x

        double new_x[2];

        new_x[0] =
            F[0][0] * x[0] +
            F[0][1] * x[1];

        new_x[1] =
            F[1][0] * x[0] +
            F[1][1] * x[1];

        x[0] = new_x[0];
        x[1] = new_x[1];


        // P = F P F^T + Q

        double FP[2][2];

        for (int i = 0; i < 2; ++i)
        {
            for (int j = 0; j < 2; ++j)
            {
                FP[i][j] =
                    F[i][0] * P[0][j] +
                    F[i][1] * P[1][j];
            }
        }

        double new_P[2][2];

        for (int i = 0; i < 2; ++i)
        {
            for (int j = 0; j < 2; ++j)
            {
                new_P[i][j] =
                    FP[i][0] * F[j][0] +
                    FP[i][1] * F[j][1] +
                    Q[i][j];
            }
        }

        for (int i = 0; i < 2; ++i)
            for (int j = 0; j < 2; ++j)
                P[i][j] = new_P[i][j];
    }


    // Update
    void update(double z)
    {
        // --------------------------------
        // 1. 计算预测观测
        // --------------------------------

        // z_hat = H * x
        //
        // H = [1 0]
        //
        // 所以：
        //
        // z_hat = position

        double z_hat =
            H[0] * x[0] +
            H[1] * x[1];


        // --------------------------------
        // 2. innovation
        // --------------------------------

        double y = z - z_hat;


        // --------------------------------
        // 3. 计算 S
        // --------------------------------
        //
        // S = H P H^T + R
        //
        // 因为 H = [1 0]
        //
        // S = P00 + R

        double S =
            H[0] * P[0][0] * H[0] +
            H[0] * P[0][1] * H[1] +
            H[1] * P[1][0] * H[0] +
            H[1] * P[1][1] * H[1] +
            R;


        // --------------------------------
        // 4. Kalman Gain
        // --------------------------------
        //
        // K = P H^T S^-1

        double K[2];

        K[0] =
            (P[0][0] * H[0] +
             P[0][1] * H[1]) / S;

        K[1] =
            (P[1][0] * H[0] +
             P[1][1] * H[1]) / S;


        // --------------------------------
        // 5. 更新状态
        // --------------------------------
        //
        // x = x + K * y

        x[0] += K[0] * y;
        x[1] += K[1] * y;


        // --------------------------------
        // 6. 更新协方差
        // --------------------------------
        //
        // P = (I - K H) P

        double I_KH[2][2];

        I_KH[0][0] = 1.0 - K[0] * H[0];
        I_KH[0][1] = 0.0 - K[0] * H[1];

        I_KH[1][0] = 0.0 - K[1] * H[0];
        I_KH[1][1] = 1.0 - K[1] * H[1];

        double new_P[2][2];

        for (int i = 0; i < 2; ++i)
        {
            for (int j = 0; j < 2; ++j)
            {
                new_P[i][j] =
                    I_KH[i][0] * P[0][j] +
                    I_KH[i][1] * P[1][j];
            }
        }

        for (int i = 0; i < 2; ++i)
            for (int j = 0; j < 2; ++j)
                P[i][j] = new_P[i][j];
    }
};


int main()
{
    KalmanFilter kf(1.0);

    // 模拟 GPS 数据
    double gps[] = {
        0.2,
        0.8,
        2.3,
        2.7,
        4.4,
        4.8,
        6.5,
        6.9,
        8.2,
        9.1
    };

    std::cout
        << std::fixed
        << std::setprecision(3);

    for (double measurement : gps)
    {
        // 预测
        kf.predict();

        // GPS观测更新
        kf.update(measurement);

        std::cout
            << "GPS = "
            << measurement
            << "  position = "
            << kf.x[0]
            << "  velocity = "
            << kf.x[1]
            << '\n';
    }
}