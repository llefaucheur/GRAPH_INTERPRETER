% Example usage
function grok_async_data()
    % Example asynchronous samples: [time_diff, data]
global T0
global T1
global W
    T0 = 1/11; T1 = 1/30;
    %T0 = 1/30; T1 = 1/11;
    W = 1;
    jitter_percent = 0.002;
    close all;
    N = 1000;
    samples = zeros(N,2);
    
    cumul_t = 0;
    for k=1:N
        delta_t = max(T0/10, T0 + jitter_percent*randn(1)/T0);
        cumul_t = cumul_t + delta_t;
        samples(k,1) = delta_t;
        samples(k,2) = sin(2*pi*cumul_t);
    end    
    
    % Desired regular interval for downsampling (seconds)
    target_interval = T1; % Lower data rate than original
    
    % Call interpolation function
    interpolated = interpolate_async_quadratic(samples, target_interval);
    
    % Convert struct to matrix for display
    out_times = cell2mat({interpolated.time}');
    out_data = cell2mat({interpolated.data}');
    
    % Display results
%     fprintf('Quadratic interpolated samples at %.2fs intervals:\n', target_interval);
%     fprintf('Time\t\tData\n');
%     for i = 1:length(out_data)
%         fprintf('%.2f\t%.2f\n', out_times(i), out_data(i));
%     end
    
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
    plot (samples(:,1)); hold on; grid on;
    mean(samples(:,1))
    std(samples(:,1))
    
    figure;
    Z = samples(:,2); tablen = length(Z); w = window(@blackmanharris,tablen); b1 = Z' .* w'; 
    F = abs(fft(b1)); F = F/max(F); ZFFT=20*log10(F); plot (ZFFT); grid on; hold on;
    Z = out_data; tablen = length(Z); w = window(@blackmanharris,tablen); b1 = Z' .* w'; 
    F = abs(fft(b1)); F = F/max(F); ZFFT=20*log10(F); plot (ZFFT,'k'); grid on; hold on;    
end


% Interpolate asynchronous data-rate samples using second-order Lagrange polynomial
function interpolated = interpolate_async_quadratic(samples, target_interval)
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
    num_samples = length(data);
    
    % Calculate output times
    total_time = sum(time_diff);
    out_times = (0:target_interval:total_time)';
    if out_times(end) < total_time
        out_times = [out_times; total_time];
    end
    num_out = length(out_times);
    
    % Initialize output data
    out_data = zeros(num_out, 1);
    
    % Perform quadratic Lagrange interpolation
    sample_idx = 2; % Start with second point to allow 3-point interpolation (needs previous and next)
    for i = 1:num_out
        current_time = out_times(i);
        
        % Find the bracketing samples
        while sample_idx < num_samples - 1 && abs_times(sample_idx + 1) < current_time
            sample_idx = sample_idx + 1;
        end
        
        % Handle edge cases
        if num_samples < 3
            % Not enough points for quadratic interpolation, use nearest point
            if current_time < abs_times(1)
                out_data(i) = data(1);
            else
                out_data(i) = data(end);
            end
        elseif current_time <= abs_times(1)
            % Before first sample: use first data point
            out_data(i) = data(1);
        elseif current_time >= abs_times(end)
            % After last sample: use last data point
            out_data(i) = data(end);
        elseif sample_idx == 1
            % Near first sample but after it: use first three points
            x0 = abs_times(1);
            x1 = abs_times(2);
            x2 = abs_times(3);
            y0 = data(1);
            y1 = data(2);
            y2 = data(3);
            
            % Lagrange basis polynomials
            l0 = ((current_time - x1) * (current_time - x2)) / ((x0 - x1) * (x0 - x2));
            l1 = ((current_time - x0) * (current_time - x2)) / ((x1 - x0) * (x1 - x2));
            l2 = ((current_time - x0) * (current_time - x1)) / ((x2 - x0) * (x2 - x1));
            
            % Interpolated value
            out_data(i) = y0 * l0 + y1 * l1 + y2 * l2;
        else
            % General case: quadratic interpolation using three points
            x0 = abs_times(sample_idx - 1);
            x1 = abs_times(sample_idx);
            x2 = abs_times(sample_idx + 1);
            y0 = data(sample_idx - 1);
            y1 = data(sample_idx);
            y2 = data(sample_idx + 1);
            
            % Lagrange basis polynomials
            l0 = ((current_time - x1) * (current_time - x2)) / ((x0 - x1) * (x0 - x2));
            l1 = ((current_time - x0) * (current_time - x2)) / ((x1 - x0) * (x1 - x2));
            l2 = ((current_time - x0) * (current_time - x1)) / ((x2 - x0) * (x2 - x1));
            
            % Interpolated value
            out_data(i) = y0 * l0 + y1 * l1 + y2 * l2;
        end
    end
    
    % Create output struct
    interpolated = struct('time', num2cell(out_times), 'data', num2cell(out_data));
end

