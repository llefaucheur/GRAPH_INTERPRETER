function lagrange_test
% Function to test the routine lagrange_new 

xk = [0,3,5,7.5,10];    % Support points to be interpolated
yk = [1,5,3.2,-2.5,2];  % The values at the points xk
x = 0:0.2:10;           % The interpolation points

N=3;
xk = (randn(1,N) ); xk = sort(xk)
yk = (randn(1,N) ) %yk = sort(yk)
x = xk(1)-0.5: 1/10: xk(N)+0.5;

% ------ Interpolation with lagrange_new routine
[y,L] = lagrange_new(xk, yk, x);

[x' y' L]

close all;
figure(1);    clf; 
plot(x,y,'r');      hold on;
stem(xk,yk,'k-','LineWidth',1);    grid on;
title('   Points to be interpolated and the interpolated function');
xlabel('x');     ylabel('y');

% figure(2);    clf;
% plot(x,L,'k-','LineWidth',1);      grid on;
% title(' The functions l_k(x)');    hold on;
% 
% La = axis; 
% for n = 1:length(xk)
%    plot([xk(n), xk(n)], [La(3),La(4)],'k--');
% end   
% plot([La(1), La(2)], [1,1],'k--');

%################
function [y,L] = lagrange_new(xk,yk,x)
% From MATLAB Central by Qazi Ejaz
% https://de.mathworks.com/matlabcentral/fileexchange/
%                 45855-lagrange-interpolation?s_tid=srchtitle

nk = length(xk);
nx = length(x);
L = ones(nx,nk);
for j = 1:nk
    for i = 1:nk
        if (i ~= j)
           L(:,j) = L(:,j).*(x' - xk(i))/(xk(j)-xk(i));
        end
    end
end
y = yk*L';