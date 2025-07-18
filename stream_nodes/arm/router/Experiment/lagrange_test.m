function lagrange_test
% Function to test the routine lagrange_new 

xk = [0,3,5,7.5,10];    % Support points to be interpolated
yk = [1,5,3.2,-2.5,2];  % The values at the points xk
x = 0:0.2:10;           % The interpolation points

N=3;
xk = (randn(1,N) ); xk = sort(xk)
yk = (randn(1,N) ) %yk = sort(yk)
x = xk(1)-0.5: 1/100: xk(N)+0.5;

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
% https://de.mathworks.com/matlabcentral/fileexchange/45855-lagrange-interpolation?s_tid=srchtitle
% function yi = lagrange(x,y,xi)
% if nargin < 3
%     x=input('x=');
%     y=input('y=');
%     xi=input('xi=');
% end
% n = length(x);
% ni = length(xi);
% L = ones(ni,n);
% for j = 1:n
%     for i = 1:n
%         if (i ~= j)
%             L(:,j) = L(:,j).*(xi' - x(i))/(x(j)-x(i));
%         end
%     end
% end
% yi = y*L';
%                 

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

%------------------------------------------------------------------------------
% https://fr.mathworks.com/matlabcentral/fileexchange/899-lagrange-polynomial-interpolation?s_tid=srchtitle
% 
%function y=lagrange(x,pointx,pointy)
%
%LAGRANGE   approx a point-defined function using the Lagrange polynomial interpolation
%
%      LAGRANGE(X,POINTX,POINTY) approx the function definited by the points:
%      P1=(POINTX(1),POINTY(1)), P2=(POINTX(2),POINTY(2)), ..., PN(POINTX(N),POINTY(N))
%      and calculate it in each elements of X
%
%      If POINTX and POINTY have different number of elements the function will return the NaN value
%
%      function wrote by: Carlo Castoldi carlo.castoldi(at)gmail.com
%      7-oct-2001
%
% n=size(pointx,2);
% L=ones(n,size(x,2));
% if (size(pointx,2)~=size(pointy,2))
%    fprintf(1,'\nERROR!\nPOINTX and POINTY must have the same number of elements\n');
%    y=NaN;
% else
%    for i=1:n
%       for j=1:n
%          if (i~=j)
%             L(i,:)=L(i,:).*(x-pointx(j))/(pointx(i)-pointx(j));
%          end
%       end
%    end
%    y=0;
%    for i=1:n
%       y=y+pointy(i)*L(i,:);
%    end
% end
%------------------------------------------------------------------------------
% 
% clc
% clear
% x=input('Enter the values of independent variable x in an array: \n');
% %x= [6.54 6.58 6.59 6.61 6.64];
% % Write the values of independent variable x.
% y=input('Enter the values of dependent variable y in an array: \n');
% %y =[2.8156 2.8182 2.8189 2.8202 2.8222]; 
% % Write the values of dependent variable y.
% xf=input('Enter the value of x where we want to find the value of f(x): ');
% n=length(x); % Number of terms of X or Y
% L=zeros(1,n);
% % For Lagrange's function.
% %Formula: f(x)?[(x-x2)(x-x3)...(x-xn)]/[(x1-x2)(x1-x3)...(x1-xn)]*y1
% % +[(x-x1)(x-x3)...(x-xn)]/[(x2-x1)(x2-x3)...(x2-xn)]*y2
% %+.....+[(x-x1)(x-x2)...(x-x(n-1))]/[(xn-x1)(xn-x2)...(xn-x(n-1))]*yn
% p=1;q=1;
% for r=1:n %Finding Lagrangian coefficients.
%     for k=1:n
%        if r~=k
%         p= p*(xf-x(k));
%         q=q*(x(r)-x(k));        
%        end      
%     end
%      L(1,r)=p/q*y(r);
%      p=1;q=1;
% end
% sol=0;
% for r=1:n %Finding final result.
%    sol=sol+L(1,r); 
% end
% fprintf('The required value is f(%1.2f)= %2.4f',xf,sol);
%------------------------------------------------------------------------------
