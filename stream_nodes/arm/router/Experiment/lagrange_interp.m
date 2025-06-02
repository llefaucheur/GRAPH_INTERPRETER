
% Sample data points
x_data = [0, 1.9, 2, 3, 4];
y_data = [1, 3,   2, 5, 4];

% Points for evaluation (smooth curve)
x_interp = linspace(min(x_data), max(x_data), 100);
y_interp = lagrange_interp1(x_data, y_data, x_interp);

% Plotting
close all;
figure;
plot(x_data, y_data, 'ro', 'MarkerSize', 8, 'DisplayName', 'Data points'); hold on;
plot(x_interp, y_interp, 'b-', 'LineWidth', 1.5, 'DisplayName', 'Lagrange interpolation');
xlabel('x');
ylabel('y');
title('Lagrange Polynomial Interpolation');
legend('show');
grid on;


function y_interp = lagrange_interp1(x_data, y_data, x_interp)
    % Lagrange interpolation function
    % Inputs:
    %   x_data - vector of x data points
    %   y_data - vector of y data points (same length as x_data)
    %   x_interp - points at which to evaluate interpolation polynomial
    % Output:
    %   y_interp - interpolated values at x_interp
    
    n = length(x_data);
    y_interp = zeros(size(x_interp));
    
    for k = 1:length(x_interp)
        L = ones(1, n); % Initialize basis polynomials
        
        for i = 1:n
            for j = [1:i-1, i+1:n]
                L(i) = L(i) * (x_interp(k) - x_data(j)) / (x_data(i) - x_data(j));
            end
        end
        
        y_interp(k) = sum(y_data .* L);
    end
end

