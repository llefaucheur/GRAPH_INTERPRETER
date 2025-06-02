% Classical Taylor Interpolation on Non-uniform Points
% No symbolic math, no polyfit, no function handles.
% Explicit numerical implementation suitable for C translation

clear all;
close all;
clc;

%% Non-uniform sample points and function values (example: exp(x))
x_interp = sort([-1.8, -1.3, -1.1, -0.7, 0, 0.4, 0.9, 1.5, 2]);
num_points = length(x_interp);
y_exact = zeros(1,num_points);
for i=1:num_points
    y_exact(i) = exp(x_interp(i));  % explicit evaluation
end

%% Parameters for Taylor interpolation
a = 0.2;                  % expansion point
order = 2;              % polynomial order
neighbor_pts = order+1; % number of points used for derivative calculation

y_interp = zeros(1,num_points);

%% Find nearest points around expansion point 'a'
[~, sorted_idx] = sort(abs(x_interp - a));
nearest_idx = sort(sorted_idx(1:neighbor_pts));
x_near = zeros(1,neighbor_pts);
y_near = zeros(1,neighbor_pts);
for i=1:neighbor_pts
    x_near(i) = x_interp(nearest_idx(i));
    y_near(i) = y_exact(nearest_idx(i));
end

%% Construct Vandermonde matrix (unrolled)
V = zeros(neighbor_pts, neighbor_pts);
for row=1:neighbor_pts
    V(row,1) = 1;
    for col=2:neighbor_pts
        V(row,col) = V(row,col-1)*(x_near(row)-a);
    end
end

%% Solve linear system V*c = y_near explicitly (Gaussian elimination)
% Augmented matrix
Aug = [V, y_near'];

% Gaussian elimination (forward elimination)
for i=1:neighbor_pts-1
    for k=i+1:neighbor_pts
        factor = Aug(k,i)/Aug(i,i);
        for j=i:neighbor_pts+1
            Aug(k,j) = Aug(k,j) - factor*Aug(i,j);
        end
    end
end

% Back substitution
coeffs = zeros(1,neighbor_pts);
for i=neighbor_pts:-1:1
    coeffs(i) = Aug(i,neighbor_pts+1);
    for j=i+1:neighbor_pts
        coeffs(i) = coeffs(i) - Aug(i,j)*coeffs(j);
    end
    coeffs(i) = coeffs(i)/Aug(i,i);
end

%% Compute derivatives at expansion point from polynomial coefficients
% coeffs = [c0, c1, c2, c3, c4] for polynomial: c0 + c1*(x-a) + c2*(x-a)^2 ...
derivatives_at_a = zeros(1,order+1);
for n=0:order
    derivatives_at_a(n+1) = coeffs(n+1)*factorial_custom(n);
end

%% Perform Taylor interpolation at each point
for i=1:num_points
    x_diff = x_interp(i)-a;
    y_interp(i) = derivatives_at_a(1); % f(a)
    term = 1;
    for n=1:order
        term = term*x_diff;
        y_interp(i) = y_interp(i) + derivatives_at_a(n+1)/factorial_custom(n)*term;
    end
end

%% Plot results
figure;
plot(x_interp,y_exact,'bo-','LineWidth',2); hold on;
plot(x_interp,y_interp,'r*--','LineWidth',1.5);
plot(a, derivatives_at_a(1),'ks','MarkerFaceColor','k');
xlabel('x');
ylabel('y');
title('Taylor Interpolation (Non-uniform, explicit polyfit)');
legend('Exact e^x','Taylor Approximation','Expansion Point','Location','Best');
grid on;

disp('Interpolation complete.');

%% Supporting function: factorial (explicit, no built-in)
function f = factorial_custom(n)
    f = 1;
    for k=2:n
        f = f*k;
    end
end
