
close all
clear all

if 0 %-----------------------------------------------------------------------------
% Parameters
fs = 1000;               % Ideal sampling frequency (Hz)
Ts = 1/fs;
N = 1024;                % Number of samples
f_signal = 50;           % Signal frequency (Hz)
jitter_std = 0.001;     % Standard deviation of jitter (in seconds)

% Time vectors
t_ideal = (0:N-1)*Ts;                        % Ideal time
jitter = jitter_std * randn(1, N);           % Gaussian jitter
t_jittered = t_ideal + jitter;               % Jittered time

% Input signal
signal = sin(2*pi*f_signal*t_ideal);         % Ideal sampled signal
signal_jittered = sin(2*pi*f_signal*t_jittered); % Jittered sample values

% FIR low-pass filter design
fcut = 100; % Cutoff frequency (Hz)
order = 50;
b = fir1(order, fcut/(fs/2));  % Normalized cutoff for FIR

% Filter responses
y_clean = filter(b, 1, signal);
y_jittered = filter(b, 1, signal_jittered);

% FFT for spectrum analysis
f = linspace(0, fs, N);
Y_clean = abs(fft(y_clean));
Y_jittered = abs(fft(y_jittered));

% Plot time domain
figure;
subplot(2,1,1);
plot(t_ideal, 1+y_clean, '*b', t_ideal, (-1)+y_jittered, '*r');
legend('Clean', 'Jittered');
title('Filtered Signals in Time Domain');
xlabel('Time (s)');
ylabel('Amplitude'); grid on

% Plot frequency domain
subplot(2,1,2);
plot(f, Y_clean, 'b', f, Y_jittered, 'r--');
xlim([0 200]);
legend('Clean', 'Jittered');
title('Filtered Signals in Frequency Domain');
xlabel('Frequency (Hz)');
ylabel('|FFT|'); grid on
end
if 1 %-----------------------------------------------------------------------------
    % Irregular time and data
t_irreg = sort(rand(1, 200));       % Irregular time stamps in [0,1]
x_irreg = sin(2*pi*5*t_irreg) + 0.0*randn(size(t_irreg));

% Interpolation to uniform grid
t_uniform = linspace(t_irreg(1), t_irreg(end), 500);
x_uniform = interp1(t_irreg, x_irreg, t_uniform, 'pchip');  % or 'spline'

% Apply low-pass FIR filter
fs = 1 / mean(diff(t_uniform));     % Estimate sampling rate
fcut = 10;                          % Cutoff frequency
b = fir1(10, fcut/(fs/2));
x_filtered = filter(b, 1, x_uniform);
x_irreg2 = filter(b, 1, x_irreg);
% Plot
plot(t_irreg, x_irreg, '*-.', t_irreg, x_irreg2, 'b-.', t_uniform, x_filtered, 'r-.'); grid on
legend('Original samples', 'Filtered (interp)');
end
if 0 %------------------------------Lomb-Scargle periodogram---------------------------------
Fs = 1000;
f0 = 175;
f1 = 400;

t = 0:1/Fs:0.5;

wgn = randn(length(t),2)/2;

sigOrig = sin(2*pi*[f0;f1]*t)' + wgn;

tirr = t + (1/2-rand(size(t)))/Fs/2;
tirr(1) = 0;

sinIrreg = sin(2*pi*[f0;f1]*tirr)' + wgn;

plomb(sinIrreg,tirr)

axis(axisLim)
title('Nonuniform Sampling')
end