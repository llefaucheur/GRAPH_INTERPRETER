if 0
% Enhanced Kalman filter implementation demonstrating its performance

% Initialization
close all; clear all;
x_est = [0; 0]; % initial state [position; velocity]
P = eye(2);

% Define system matrices for a more challenging scenario
dt = 0.01; % sampling interval
A = [1 dt; 0 1];
B = [0.5*dt^2; dt];
C = [1 0];

% More realistic noise levels
Q = [5e-4, 0; 0, 2e-3];
R = 1e-1;

u = 0.1; % varying control input

% Simulation parameters
num_steps = 2000;
x_true = [0; 0];

NN = 3;
LPz = 0; LPc = 0.1;
YY = zeros(NN,1);

for k = 1:num_steps
    % Simulate a varying control input
    u = 0.1 * sin(0.05 * pi * k);

    % True system simulation with higher complexity
    process_noise = chol(Q, 'lower') * randn(2,1);
    x_true = A * x_true + B * u + process_noise;

    % Measurement simulation with larger noise
    measurement_noise = sqrt(R) * randn;
    y = C * x_true + measurement_noise;

    % Kalman Filter Prediction
    x_pred = A * x_est + B * u;
    P_pred = A * P * A' + Q;

    % Kalman Gain
    K = P_pred * C' / (C * P_pred * C' + R);

    % Update
    x_est = x_pred + K * (y - C * x_pred);
    P = (eye(2) - K * C) * P_pred;

    LPz = y*LPc + LPz*(1-LPc);      % LPF
    YY(2:end) = YY(1:end-1);
    YY(1) = LPz;
    x_est2 = median(YY);
    
    % Logging results
    x_est_log2(:,k) = x_est2(1);
    x_est_log(:,k) = x_est(1);
    x_true_log(:,k) = x_true(1);
end


kalman_error = std(x_true_log(1,:) - x_est_log(1,:));
LLF_error = std(x_true_log(1,:) - x_est_log2(1,:));
fprintf(1,'\nKalman  %5.4f       LPF  %5.4f\n', kalman_error, LLF_error);

% Plot results
figure;
plot(1:num_steps, x_true_log(1,:), 'g', 'DisplayName', 'True Position'); hold on;
plot(1:num_steps, x_est_log(1,:), 'b--', 'DisplayName', 'Kalman Estimated Position');
plot(1:num_steps, x_est_log2(1,:), 'r', 'DisplayName', 'LLF Estimated Position');
xlabel('Time Steps'); ylabel('Position'); grid on;
legend;
title('Enhanced Kalman Filter Position Estimation');

end

%------------------------------------------------------------------------
if 1
    % Kalman filter implementation for motor control applications

    % Initialization
    clear all; close all;
    x_est = [0; 0]; % initial state [position; velocity]
    P = eye(2); % initial covariance

    % Define system matrices (example for a DC motor)
    dt = 0.01; % sampling interval
    A = [1 dt; 0 1]; % state transition model
    B = [0.5*dt^2; dt]; % control input model
    C = [1 0]; % measurement model (measuring position only)

    Q = [1e-5, 0; 0, 1e-4]; % process noise covariance
    R = 1e-2; % measurement noise covariance

    u = 1; % constant control input (e.g., voltage applied)

    % Simulation parameters
    num_steps = 2000;
    x_true = [0; 0]; % true state initialization
    
NN = 3;
LPz = 0; LPc = 0.5;
YY = zeros(NN,1);    

    for k = 1:num_steps
        % True system simulation
        process_noise = chol(Q, 'lower') * randn(2,1);
        x_true = A * x_true + B * u + process_noise;

        % Measurement simulation
        measurement_noise = sqrt(R) * randn;
        y = C * x_true + measurement_noise;

        % Kalman Filter Prediction
        x_pred = A * x_est + B * u;
        P_pred = A * P * A' + Q;

        % Kalman Gain
        K = P_pred * C' / (C * P_pred * C' + R);

        % Update
        x_est = x_pred + K * (y - C * x_pred);
        P = (eye(2) - K * C) * P_pred;

    LPz = y*LPc + LPz*(1-LPc);      % LPF
    YY(2:end) = YY(1:end-1);
    YY(1) = LPz;
    x_est2 = median(YY);
    
    % Logging results
    x_est_log2(:,k) = x_est2(1);
        x_est_log(:,k) = x_est;
        x_true_log(:,k) = x_true;
        y_log(:,k) = y;
        
        
    end

    % Plot results
    figure;
    plot(1:num_steps, x_true_log(1,:), 'g', 'DisplayName', 'True Position'); hold on;
    plot(1:num_steps, x_est_log(1,:), 'b--', 'DisplayName', 'Estimated Position');
plot(1:num_steps, x_est_log2(1,:), 'k', 'DisplayName', 'LLF Estimated Position');    
plot(1:num_steps, y_log(1,:), 'r', 'DisplayName', 'Y');    
    xlabel('Time Steps'); ylabel('Position');
    legend;
    title('Kalman Filter Position Estimation');
    
kalman_error = std(x_true_log(1,:) - x_est_log(1,:));
LLF_error = std(x_true_log(1,:) - x_est_log2(1,:));
fprintf(1,'\nKalman  %5.4f       LPF  %5.4f\n', kalman_error, LLF_error);    
    
    
end