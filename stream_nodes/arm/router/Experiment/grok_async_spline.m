% Example usage
function grok_async_data()
    % Example asynchronous samples: [time_diff, data]
    samples = [
        0.0, 10.0;  % First sample at t=0
        0.3, 12.0;  % 0.3s later
        0.5, 15.0;  % 0.5s after previous
        0.2, 14.0;  % 0.2s after previous
        0.4, 16.0   % 0.4s after previous
    ];

global T0
global T1
global W
    T0 = 0.08;
    T1 = 0.07;
    W = 1;
    close all;
    N = 200;
    samples = zeros(N,2);
    for k=1:N
        samples(k,1) = T0;  %max(T0/10, T0 + randn(1)/T0/200);
        samples(k,2) = sin(2*pi*k/7);
    end    
    
    % Desired regular interval for downsampling (seconds)
    target_interval = T1; % Lower data rate than original
    
    % Call interpolation function
    interpolated = interpolate_async_custom_spline(samples, target_interval);
    
    % Convert struct to matrix for display
    out_times = cell2mat({interpolated.time}');
    out_data = cell2mat({interpolated.data}');
    
    % Display results
    fprintf('Quadratic interpolated samples at %.2fs intervals:\n', target_interval);
    fprintf('Time\t\tData\n');
    for i = 1:length(out_data)
        fprintf('%.2f\t%.2f\n', out_times(i), out_data(i));
    end
    
    % Plot results
    figure;
    plot(out_times, out_data, 'ko-', 'DisplayName', 'Quadratic Interpolated');
    hold on;
    plot(cumsum([0; samples(2:end,1)]), samples(:,2), 'x-', 'DisplayName', 'Original Samples');
    xlabel('Time (s)');
    ylabel('Data');
    title('Quadratic Lagrange Interpolation of Asynchronous Samples');
    legend;
    grid on;

    
    figure;
    Z = samples(:,2); tablen = length(Z); w = window(@blackmanharris,tablen); b1 = Z' .* w'; 
    F = abs(fft(b1)); F = F/max(F); ZFFT=20*log10(F); plot (ZFFT); grid on; hold on;
    Z = out_data; tablen = length(Z); w = window(@blackmanharris,tablen); b1 = Z' .* w'; 
    F = abs(fft(b1)); F = F/max(F); ZFFT=20*log10(F); plot (ZFFT,'k'); grid on; hold on;    
end

% Interpolate asynchronous data-rate samples using custom cubic spline interpolation
function interpolated = interpolate_async_custom_spline(samples, target_interval)
    % Inputs:
    %   samples: Nx2 matrix where first column is time_diff (seconds), second is data
    %   target_interval: Desired regular interval for interpolation (seconds)
    %
    % Output:
    %   interpolated: Struct array with fields 'time' and 'data'

    % Extract time differences and data
    time_diff = samples(:,1);
    data = samples(:,2);
    
    % Calculate absolute times
    abs_times = cumsum([0; time_diff(2:end)]);
    n = length(data);
    
    % Calculate output times
    total_time = sum(time_diff);
    out_times = (0:target_interval:total_time)';
    if out_times(end) < total_time
        out_times = [out_times; total_time];
    end
    num_out = length(out_times);
    
    % Initialize output data
    out_data = zeros(num_out, 1);
    
    % Compute cubic spline coefficients (natural spline: second derivatives = 0 at endpoints)
    if n < 2
        % Not enough points for interpolation, return original data
        out_data = data(1) * ones(num_out, 1);
    else
        % Step 1: Set up tridiagonal system for second derivatives
        h = diff(abs_times); % Interval lengths
        b = zeros(n, 1); % Right-hand side for tridiagonal system
        for i = 2:n-1
            b(i) = 6 * ((data(i+1) - data(i)) / h(i) - (data(i) - data(i-1)) / h(i-1));
        end
        
        % Tridiagonal matrix coefficients
        diag = zeros(n, 1);
        lower = zeros(n-1, 1);
        upper = zeros(n-1, 1);
        diag(1) = 1; diag(n) = 1; % Natural spline boundary conditions
        for i = 2:n-1
            diag(i) = 2 * (h(i-1) + h(i));
            lower(i-1) = h(i-1);
            upper(i) = h(i);
        end
        
        % Step 2: Solve tridiagonal system for second derivatives (m)
        m = zeros(n, 1);
        if n >= 3
            % Thomas algorithm for tridiagonal system
            c = upper; % Upper diagonal
            d = b; % Right-hand side
            for i = 2:n-1
                factor = lower(i-1) / diag(i-1);
                diag(i) = diag(i) - factor * c(i-1);
                d(i) = d(i) - factor * d(i-1);
            end
            m(n-1) = d(n-1) / diag(n-1);
            for i = n-2:-1:2
                m(i) = (d(i) - c(i) * m(i+1)) / diag(i);
            end
        end
        
        % Step 3: Interpolate at output times
        for i = 1:num_out
            current_time = out_times(i);
            
            % Find the interval [abs_times(k), abs_times(k+1)] containing current_time
            k = find(current_time >= abs_times, 1, 'last');
            if isempty(k) || k == n
                % Before first point or after last point: use nearest value
                out_data(i) = data(min(max(1, k), n));
            else
                % Cubic spline interpolation in interval [abs_times(k), abs_times(k+1)]
                t = (current_time - abs_times(k)) / h(k);
                a = (m(k) * (abs_times(k+1) - current_time)^3) / (6 * h(k));
                b = (m(k+1) * (current_time - abs_times(k))^3) / (6 * h(k));
                c = ((data(k+1) / h(k)) - (m(k+1) * h(k) / 6)) * (current_time - abs_times(k));
                d = ((data(k) / h(k)) - (m(k) * h(k) / 6)) * (abs_times(k+1) - current_time);
                out_data(i) = a + b + c + d;
            end
        end
    end
    
    % Create output struct
    interpolated = struct('time', num2cell(out_times), 'data', num2cell(out_data));
end
