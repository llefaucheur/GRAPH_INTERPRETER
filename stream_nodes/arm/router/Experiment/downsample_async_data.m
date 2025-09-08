% Example usage
function downsample_async_data()
    % Example asynchronous samples: [time_diff, data]
%     samples = [
%         0.0, 10.0;  % First sample at t=0
%         0.3, 12.0;  % 0.3s later
%         0.5, 15.0;  % 0.5s after previous
%         0.2, 14.0;  % 0.2s after previous
%         0.4, 16.0   % 0.4s after previous
%     ];
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
        samples(k,2) = sin(2*pi*k/10);
    end    
    
    % Desired regular interval for downsampling (seconds)
    target_interval = T1; % Lower data rate than original
    
    % Call downsampling function
    downsampled = downsample_async_data0(samples, target_interval);
    
    % Convert struct to matrix for display
    out_times = cell2mat({downsampled.time}');
    out_data = cell2mat({downsampled.data}');
    
    %w = window(@chebwin,1+2*tablen,chebatt);%b1 = b1 .* w'; 
    
    % Display results
%     fprintf('Downsampled data at %.2fs intervals (with moving average filter):\n', target_interval);
%     fprintf('Time\tData\n');
%     for i = 1:length(downsampled)
%         fprintf('%.2f\t%.2f\n', out_times(i), out_data(i));
%     end
    
    % Optional: Plot results
    figure;
    plot(out_times, out_data, 'o-', 'DisplayName', 'Downsampled');
    hold on;
    plot(cumsum([0; samples(2:end,1)]), samples(:,2), 'x-', 'DisplayName', 'Original');
    xlabel('Time (s)');
    ylabel('Data');
    title('Downsampled vs Original Data');
    legend;
    grid on;
    
    figure;
    Z = samples(:,2); tablen = length(Z); w = window(@blackmanharris,tablen); b1 = Z' .* w'; 
    F = abs(fft(b1)); F = F/max(F); ZFFT=20*log10(F); plot (ZFFT); grid on; hold on;
    Z = out_data; tablen = length(Z); w = window(@blackmanharris,tablen); b1 = Z' .* w'; 
    F = abs(fft(b1)); F = F/max(F); ZFFT=20*log10(F); plot (ZFFT,'k'); grid on; hold on;
end

% Downsample asynchronous data-rate samples using moving average filter and linear interpolation
function downsampled = downsample_async_data0(samples, target_interval)
    % Inputs:
    %   samples: Nx2 matrix where first column is time_diff (seconds), second is data
    %   target_interval: Desired regular interval for downsampling (seconds)
    %
    % Output:
    %   downsampled: Struct array with fields 'time' and 'data'

    % Extract time differences and data
global T0
global T1
global W    
    time_diff = samples(:,1);
    data = samples(:,2);
    
    % Calculate absolute times
    abs_times = cumsum([0; time_diff(2:end)]);
    
    % Apply moving average filter
    window_size = W; % Adjust based on desired smoothing (odd number)
    filtered_data = movmean(data, window_size);
    
    % Calculate output times
    total_time = sum(time_diff);
    out_times = (0:target_interval:total_time)';
    if out_times(end) < total_time
        out_times = [out_times; total_time];
    end
    
    % Perform linear interpolation on filtered data
    %interpolated_data = interp1(abs_times, filtered_data, out_times, 'linear', 'extrap');
    interpolated_data = interp1(abs_times, filtered_data, out_times, 'cubic', 'extrap');
    %interpolated_data = interp1(abs_times, filtered_data, out_times, 'spline', 'extrap');
    
    % Create output struct
    downsampled = struct('time', num2cell(out_times), 'data', num2cell(interpolated_data));
end

