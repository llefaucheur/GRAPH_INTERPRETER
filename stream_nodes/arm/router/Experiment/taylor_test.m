% Classical Taylor Polynomial Interpolation
% Explicit numeric derivatives computed directly from sampled points
% Example with f(x) = exp(x) without symbolic or function handles

clear all;
close all;
clc;

%% Parameters
a = 0;                % Expansion point
order = 4;            % Taylor polynomial order
x_min = -2;           
x_max = 2;
num_points = 100;

%% Generate interpolation points
x_interp = linspace(x_min, x_max, num_points);
y_exact = exp(x_interp);  % known function values for demonstration
y_interp = zeros(1, num_points);

%% Compute numeric derivatives at expansion point 'a' using finite differences
h = x_interp(2) - x_interp(1); % spacing between interpolation points
derivatives_at_a = zeros(1, order+1); % derivatives from 0th (function value) to nth

% Find index of expansion point in x_interp
[~, a_index] = min(abs(x_interp - a));

% Check if expansion point is within the array bounds
if (a_index - order < 1) || (a_index + order > num_points)
    error('Expansion point too close to boundary for chosen polynomial order.');
end

% 0th derivative (function value)
derivatives_at_a(1) = y_exact(a_index);

% Compute higher-order derivatives numerically using central differences
for n = 1:order
    derivatives_at_a(n+1) = central_difference(y_exact, a_index, n, h);
end

%% Compute Taylor interpolation at each point
for i = 1:num_points
    x_diff = x_interp(i) - a;
    y_interp(i) = derivatives_at_a(1); % start with f(a)
    
    for n = 1:order
        y_interp(i) = y_interp(i) + (derivatives_at_a(n+1) / factorial_custom(n)) * (x_diff)^n;
    end
end

%% Plot results
figure;
plot(x_interp, y_exact, 'b-', 'LineWidth', 2); hold on;
plot(x_interp, y_interp, 'r--', 'LineWidth', 2);
plot(a, derivatives_at_a(1), 'ko', 'MarkerFaceColor', 'k');
xlabel('x');
ylabel('y');
title('Classical Numeric Taylor Polynomial Interpolation');
legend('Exact e^x', 'Taylor Approximation', 'Expansion Point');
grid on;

%% Display completion message
disp('Interpolation complete.');

%% Supporting function: central difference numeric derivative
function deriv = central_difference(y_values, idx, n, h)
    % Compute nth derivative using central difference
    if n == 1
        deriv = (y_values(idx + 1) - y_values(idx - 1)) / (2*h);
    elseif n == 2
        deriv = (y_values(idx + 1) - 2*y_values(idx) + y_values(idx - 1)) / (h^2);
    elseif n == 3
        deriv = (y_values(idx + 2) - 2*y_values(idx + 1) + 2*y_values(idx - 1) - y_values(idx - 2)) / (2*h^3);
    elseif n == 4
        deriv = (y_values(idx + 2) - 4*y_values(idx + 1) + 6*y_values(idx) - 4*y_values(idx - 1) + y_values(idx - 2)) / (h^4);
    else
        error('Derivative order higher than 4 not implemented.');
    end
end

%% Supporting function: factorial (no built-in function)
function f = factorial_custom(n)
    f = 1;
    for k = 2:n
        f = f * k;
    end
end
