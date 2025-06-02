#define SPLINES2            0
#define TAYLOR_3            0
#define TAYLOR_2            0
#define LAGRANGE            1
#define NEVILLE             0
#define TAYLOR_1            0
#define taylor_nonuniform   0 
#define NEVILLE3            0
#define SPLINES             0

#if taylor_nonuniform
#include <stdio.h>
#include <math.h>

#define MAX_POINTS 10
#define ORDER 2

// Factorial function
int factorial(int n){
    int f = 1;
    for(int i=2; i<=n; i++)
        f *= i;
    return f;
}

// Example function to interpolate
double func(double x){
    return exp(x);
}

int main(void){
    
    // Non-uniform interpolation points (example)
    int num_points = 9;
    double x_interp[MAX_POINTS] = {-1.8, -1.3, -1.1, -0.7, 0.1, 0.9, 1.5, 2.0, 3.0};
    double y_exact[MAX_POINTS];

    // Compute function values explicitly
    for(int i=0; i<num_points; i++)
        y_exact[i] = func(x_interp[i]);

    // Taylor parameters
    double expansion_point = 0.2; // Expansion point
    int neighbor_pts = ORDER + 1;

    // Find nearest points around 'a'
    int idx_near[ORDER+1] = {0};
    double x_near[ORDER+1] = {0};
    double y_near[ORDER+1] = {0};

    // Simple selection of nearest points
    int idx_sorted[MAX_POINTS];
    for(int i=0; i<num_points; i++)
        idx_sorted[i] = i;

    // Sorting indices by distance (simple bubble sort)
    for(int i=0; i<num_points-1; i++){
        for(int j=0; j<num_points-1-i; j++){
            if(fabs(x_interp[idx_sorted[j]]-expansion_point) > fabs(x_interp[idx_sorted[j+1]]-expansion_point)){
                int temp = idx_sorted[j];
                idx_sorted[j] = idx_sorted[j+1];
                idx_sorted[j+1] = temp;
            }
        }
    }

    // Select nearest neighbor_pts points
    for(int i=0; i<neighbor_pts; i++){
        idx_near[i] = idx_sorted[i];
        x_near[i] = x_interp[idx_near[i]];
        y_near[i] = y_exact[idx_near[i]];
    }

    // Construct Vandermonde matrix explicitly
    double V[ORDER+1][ORDER+1];
    for(int row=0; row<neighbor_pts; row++){
        V[row][0] = 1.0;
        for(int col=1; col<neighbor_pts; col++)
            V[row][col] = V[row][col-1] * (x_near[row] - expansion_point);
    }

    // Augmented matrix (V|y_near)
    double Aug[ORDER+1][ORDER+2];
    for(int i=0; i<neighbor_pts; i++){
        for(int j=0; j<neighbor_pts; j++)
            Aug[i][j] = V[i][j];
        Aug[i][neighbor_pts] = y_near[i];
    }

    // Gaussian elimination (forward elimination)
    for(int i=0; i<neighbor_pts-1; i++){
        for(int k=i+1; k<neighbor_pts; k++){
            double factor = Aug[k][i]/Aug[i][i];
            for(int j=i; j<=neighbor_pts; j++)
                Aug[k][j] -= factor * Aug[i][j];
        }
    }

    // Back substitution
    double coeffs[ORDER+1];
    for(int i=neighbor_pts-1; i>=0; i--){
        coeffs[i] = Aug[i][neighbor_pts];
        for(int j=i+1; j<neighbor_pts; j++)
            coeffs[i] -= Aug[i][j]*coeffs[j];
        coeffs[i] /= Aug[i][i];
    }

    // Compute derivatives from coefficients
    double derivatives[ORDER+1];
    for(int n=0; n<=ORDER; n++)
        derivatives[n] = coeffs[n]*factorial(n);

    // Taylor interpolation at each original point
    double y_interp[MAX_POINTS];
    for(int i=0; i<num_points; i++){
        double x_diff = x_interp[i]-expansion_point;
        double term = 1.0;
        y_interp[i] = derivatives[0]; // f(a)
        for(int n=1; n<=ORDER; n++){
            term *= x_diff;
            y_interp[i] += derivatives[n]/factorial(n)*term;
        }
    }

    // Display results
    printf(" x\t   exact f(x)\t Taylor approx\t  Error%%\n");
    printf("--------------------------------------------------\n");
    for(int i=0; i<num_points; i++){
        printf("%5.2f\t%12.6f\t%12.6f\t%10.6f\n", 
            x_interp[i], y_exact[i], y_interp[i], 0.01 * fabs(y_exact[i]-y_interp[i])/fabs(y_exact[i]));
    }

    return 0;
}

#endif 

#if SPLINES2
#include <stdio.h>

/* Maximum number of data points */
#define MAX_POINTS 100

/* Computes cubic spline second derivatives (natural spline) */
void spline_coeff(int n, double t[], double y[], double m[])
{
    int i, j;
    double h[MAX_POINTS], alpha[MAX_POINTS], l[MAX_POINTS], mu[MAX_POINTS], z[MAX_POINTS];

    /* Compute intervals h[i] */
    for(i = 0; i < n-1; i++)
        h[i] = t[i+1] - t[i];

    /* Compute alpha[i] */
    alpha[0] = 0.0; /* unused */
    for(i = 1; i < n-1; i++)
        alpha[i] = (3.0/h[i])*(y[i+1]-y[i]) - (3.0/h[i-1])*(y[i]-y[i-1]);

    /* Set up tridiagonal system */
    l[0] = 1.0; mu[0] = z[0] = 0.0;
    for(i = 1; i < n-1; i++){
        l[i] = 2.0*(t[i+1]-t[i-1]) - h[i-1]*mu[i-1];
        mu[i] = h[i]/l[i];
        z[i] = (alpha[i] - h[i-1]*z[i-1])/l[i];
    }

    l[n-1] = 1.0; z[n-1] = m[n-1] = 0.0;

    /* Back substitution */
    for(j = n-2; j >= 0; j--)
        m[j] = z[j] - mu[j]*m[j+1];
}

/* Evaluates the spline at point tq */
double spline_eval(int n, double t[], double y[], double m[], double tq)
{
    int i = 0;
    double h, A, B, yq;

    /* Find correct interval */
    while(i < n-2 && tq > t[i+1])
        i++;

    h = t[i+1] - t[i];
    A = (t[i+1] - tq)/h;
    B = (tq - t[i])/h;

    /* Cubic spline polynomial evaluation */
    yq = A*y[i] + B*y[i+1] + ((A*A*A - A)*m[i] + (B*B*B - B)*m[i+1])*(h*h)/6.0;

    return yq;
}

int main(void)
{
    /* Input samples (non-uniform spacing) */
    double t[] = {0.0, 1.0, 2.2, 3.1, 4.8, 6.0};
    double y[] = {0.0, 0.8415, 0.8085, 0.0416, -0.9962, -0.2794}; /* sin(t) values */
    double m[MAX_POINTS]; /* second derivatives */
    int n = 6;
    double tq, tq_start, tq_end, step;
    FILE *fout;

    /* Compute spline coefficients */
    spline_coeff(n, t, y, m);

    /* Output results to CSV file */
    fout = fopen("output_spline.csv", "w");
    if (fout == NULL) {
        printf("Error opening file for output.\n");
        return 1;
    }

    fprintf(fout, "t,y_interp\n");

    /* Interpolation range and step */
    tq_start = t[0];
    tq_end = t[n-1];
    step = 0.05;

    for(tq = tq_start; tq <= tq_end; tq += step){
        fprintf(fout, "%f,%f\n", tq, spline_eval(n, t, y, m, tq));
    }

    fclose(fout);
    printf("Interpolation results saved to output_spline.csv\n");

    return 0;
}


#endif

#if TAYLOR_2
#include <stdio.h>
#include <stdlib.h>

#define OUTPUT_INTERVAL 0.1
#define INPUT_INTERVAL 0.2

typedef struct {
    double t;    // timestamp
    double y;    // value
    double d1;   // first derivative
    double d2;   // second derivative
} Sample;

Sample buf[3];  // buffer for 2nd-order (3 points)
int buf_count = 0;

// Numerical derivatives (central difference)
void compute_derivatives() {
    if (buf_count < 3) return; // Need 3 points for second-order

    double t_prev = buf[0].t, t_curr = buf[1].t, t_next = buf[2].t;
    double y_prev = buf[0].y, y_curr = buf[1].y, y_next = buf[2].y;

    double dt_prev = t_curr - t_prev;
    double dt_next = t_next - t_curr;

    // First derivative at buf[1] (central difference)
    buf[1].d1 = (y_next - y_prev) / (dt_prev + dt_next);

    // Second derivative at buf[1] (central second difference)
    buf[1].d2 = (y_next - 2*y_curr + y_prev) / ((0.5*(dt_prev + dt_next))*(0.5*(dt_prev + dt_next)));
}

// Second-order Taylor interpolation
double taylor_interp(Sample s, double t) {
    double dt = t - s.t;
    return s.y + s.d1*dt + 0.5*s.d2*dt*dt;
}

int main() {
    double next_output_time = 0.0, input_t = 0, input_y = 0, jitter;
    int initialized = 0, N = 100;

    printf("Enter input samples as pairs of <time value>, Ctrl+D to finish:\n");

    //while (scanf("%lf %lf", &input_t, &input_y) == 2) {
    while (N > 0) {
        input_t += INPUT_INTERVAL;
        jitter = ((double)rand() / RAND_MAX) * (INPUT_INTERVAL/0.5); // jitter
        input_t += jitter;
        input_y = input_t /2;

        // shift buffer
        if (buf_count < 3) buf_count++;
        for (int i = 0; i < 2; i++)
            buf[i] = buf[i+1];
        buf[buf_count - 1].t = input_t;
        buf[buf_count - 1].y = input_y;

        if (buf_count < 3) continue; // wait until buffer fills

        compute_derivatives();

        // initialize output timing
        if (!initialized) {
            next_output_time = buf[1].t;
            initialized = 1;
        }

        // Generate outputs asynchronously
        while (next_output_time <= buf[2].t) {
            double interp_val = taylor_interp(buf[1], next_output_time);
            printf("Output @ %.3f s: %.6f\n", next_output_time, interp_val);
            next_output_time += OUTPUT_INTERVAL;

            N--;
        }
    }

    return 0;
}

#endif


#if TAYLOR_3

#define FIXED_BUFFER_SIZE 0
#if FIXED_BUFFER_SIZE       // FIXED_BUFFER_SIZE==1
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_SAMPLES 100
#define OUTPUT_INTERVAL 0.1
#define SIMULATION_END 30.0

// Structure for samples
typedef struct {
    double t; // timestamp
    double y; // value
    double d1, d2, d3; // derivatives
} Sample;

// Generate jittered input samples
int generate_input(Sample samples[]) {
    int count = 0;
    double t = 0.0;
    double interval, jitter;

    srand(42); // reproducibility

    while (t < SIMULATION_END && count < MAX_SAMPLES) {
        jitter = ((double)rand() / RAND_MAX) * 0.1; // 0 to 0.1s jitter
        interval = 1.0 + jitter;
        t += interval;

        samples[count].t = t;
        samples[count].y = t/2.0; //sin(t);
        count++;
    }

    return count;
}

// Compute numerical derivatives (central differences)
void compute_derivatives(Sample samples[], int count) {
    for (int i = 1; i < count - 1; i++) {
        double dt_prev = samples[i].t - samples[i - 1].t;
        double dt_next = samples[i + 1].t - samples[i].t;

        // First derivative
        samples[i].d1 = (samples[i + 1].y - samples[i - 1].y) / (dt_prev + dt_next);

        // Second derivative
        samples[i].d2 = (samples[i + 1].y - 2 * samples[i].y + samples[i - 1].y) / 
                        ((dt_prev + dt_next) / 2 * (dt_prev + dt_next) / 2);

        // Third derivative (approximation)
        if (i > 1 && i < count - 2) {
            double dt_total = samples[i + 1].t - samples[i - 1].t;
            samples[i].d3 = (samples[i + 2].y - 3*samples[i + 1].y + 3*samples[i].y - samples[i - 1].y) /
                            pow(dt_total / 3, 3);
        } else {
            samples[i].d3 = 0; // Edge cases: set to 0
        }
    }
    // Boundary derivatives set to zero for simplicity
    samples[0].d1 = samples[0].d2 = samples[0].d3 = 0;
    samples[count - 1].d1 = samples[count - 1].d2 = samples[count - 1].d3 = 0;
}

// 3rd-order Taylor interpolation
double taylor_interpolate(Sample s, double t) {
    double dt = t - s.t;
    return s.y + s.d1 * dt + (s.d2 * dt * dt) / 2.0 + (s.d3 * dt * dt * dt) / 6.0;
}

// Main simulation
int main() {
    Sample samples[MAX_SAMPLES];
    int input_count = generate_input(samples);
    compute_derivatives(samples, input_count);

    double output_time;
    int input_idx = 0;

    printf("Time(s)\tInterpolated Value\n");
    for (output_time = 0.0; output_time <= SIMULATION_END; output_time += OUTPUT_INTERVAL) {
        // Find the nearest input sample before or equal to output_time
        while ((input_idx < input_count - 1) && (samples[input_idx + 1].t <= output_time))
            input_idx++;

        if (input_idx == input_count - 1) {
            printf("%.2f\tInsufficient data\n", output_time);
            continue;
        }

        double interp_val = taylor_interpolate(samples[input_idx], output_time);
        printf("%.2f\t%.5f\n", output_time, interp_val);
    }

    return 0;
}
#else // FIXED_BUFFER_SIZE = 0
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define OUTPUT_INTERVAL 0.1

typedef struct {
    double t;    // timestamp
    double y;    // value
    double d1;   // first derivative
    double d2;   // second derivative
    double d3;   // third derivative
} Sample;

Sample buf[4];  // ring buffer for latest 4 samples
int buf_count = 0;

// Numerical derivatives (finite differences)
void compute_derivatives() {
    if (buf_count < 4) return; // need 4 points

    double t0 = buf[1].t, t1 = buf[2].t, t2 = buf[3].t, t_1 = buf[0].t;
    double y0 = buf[1].y, y1 = buf[2].y, y2 = buf[3].y, y_1 = buf[0].y;

    double dt1 = t1 - t0;
    double dt2 = t2 - t1;
    double dt0 = t0 - t_1;

    // First derivative at buf[1]
    buf[1].d1 = (y1 - y_1) / (dt0 + dt1);

    // Second derivative at buf[1]
    buf[1].d2 = (y1 - 2*y0 + y_1) / (0.5*(dt0 + dt1)*(dt0 + dt1));

    // Third derivative at buf[1]
    buf[1].d3 = (y2 - 3*y1 + 3*y0 - y_1) / pow((t2 - t_1)/3.0, 3);
}

// Taylor interpolation function
double taylor_interp(Sample s, double t) {
    double dt = t - s.t;
    return s.y + s.d1*dt + (s.d2*dt*dt)/2.0 + (s.d3*dt*dt*dt)/6.0;
}

int main() {
    double next_output_time = 0.0, input_t = 0, input_y = 0;
    int initialized = 0, N = 100;

    printf("Enter input samples as pairs of <time value>, Ctrl+D to finish:\n");

    //while (scanf("%lf %lf", &input_t, &input_y) == 2) {
    while (N-- > 0) {
        input_t += 0.1;
        input_y = input_t /2;


        // shift buffer
        if (buf_count < 4) buf_count++;
        for (int i = 0; i < 3; i++)
            buf[i] = buf[i+1];
        buf[buf_count - 1].t = input_t;
        buf[buf_count - 1].y = input_y;

        if (buf_count < 4) continue; // wait for buffer to fill

        compute_derivatives();

        // initialize output timing once buffer is ready
        if (!initialized) {
            next_output_time = buf[1].t;
            initialized = 1;
        }

        // Generate outputs asynchronously
        while (next_output_time <= buf[2].t) {
            double interp_val = taylor_interp(buf[1], next_output_time);
            printf("Output @ %.3f s: %.6f\n", next_output_time, interp_val);
            next_output_time += OUTPUT_INTERVAL;
        }
    }

    return 0;
}
#endif

#endif


#if SPLINES
#include <stdio.h>
#include <math.h>

#define N 50  // Input samples: 0s to 10s, 11 points
#define OUTPUT_INTERVAL 0.1

// Simple cubic spline for equidistant points
void cubic_spline(double x[], double y[], double M[], int n) {
    double h = x[1] - x[0];  // uniform spacing
    double alpha[N], l[N], mu[N], z[N];

    // Natural spline boundary conditions
    alpha[0] = alpha[n - 1] = 0.0;
    for (int i = 1; i < n - 1; i++)
        alpha[i] = (3.0/h)*(y[i+1]-2*y[i]+y[i-1]);

    l[0] = 1.0; mu[0] = z[0] = 0.0;
    for (int i = 1; i < n - 1; i++) {
        l[i] = 4*h - h*mu[i-1];
        mu[i] = h / l[i];
        z[i] = (alpha[i] - h*z[i-1]) / l[i];
    }

    l[n - 1] = 1.0; z[n - 1] = M[n - 1] = 0.0;

    for (int j = n - 2; j >= 0; j--)
        M[j] = z[j] - mu[j]*M[j+1];
}

// Evaluate spline at point t
double spline_eval(double t, double x[], double y[], double M[], int n) {
    int i = (int)((t - x[0]) / (x[1]-x[0]));
    if(i < 0) i = 0;
    if(i >= n-1) i = n-2;
    
    double h = x[i+1] - x[i];
    double A = (x[i+1] - t)/h;
    double B = (t - x[i])/h;

    return A*y[i] + B*y[i+1] + ((pow(A,3)-A)*M[i] + (pow(B,3)-B)*M[i+1])*(h*h)/6.0;
}

int main() {
    double x[N], y[N], M[N];
    double J = 1;
    for (int i = 0; i < N; i++) {
        x[i] = J; //(double)i;
        J = J * 1.3;
        y[i] = x[i]/2; // sin(x[i]);
    }

    cubic_spline(x, y, M, N);

    for (double t = 0.0; t <= 10.0; t += OUTPUT_INTERVAL) {
        double interpolated = spline_eval(t, x, y, M, N);
        printf("Spline Output @ %.2f s: %.5f\n", t, interpolated);
    }

    return 0;
}

#endif

#if NEVILLE3
#include <stdio.h>
#include <math.h>

#define INPUT_INTERVAL 1.0
#define OUTPUT_INTERVAL 0.1

// Neville's polynomial interpolation (order 3 requires 4 points)
double neville(double t, double x[], double y[], int n) {
    double Q[4][4];
    for (int i = 0; i < n; i++)
        Q[i][0] = y[i];

    for (int j = 1; j < n; j++) {
        for (int i = 0; i < n - j; i++) {
            Q[i][j] = ((t - x[i + j]) * Q[i][j - 1] +
                      (x[i] - t) * Q[i + 1][j - 1]) /
                      (x[i] - x[i + j]);
        }
    }

    return Q[0][n - 1];
}

int main() {
    double duration = 10.0;
    double next_output_time = 0.0;

    // 4-point window (previous 1s, current, next two future points)
    double input_times[4];
    double input_values[4];

    // Initialize first 4 points
    for (int i = 0; i < 4; i++) {
        input_times[i] = i * INPUT_INTERVAL;
        input_values[i] = input_times[i]/2; //sin(input_times[i]);
    }

    int current_idx = 1; // Points at input_times[1] and input_times[2] are around current_time

    while (next_output_time <= duration) {
        // Shift window if necessary
        if (next_output_time > input_times[current_idx + 1]) {
            for (int i = 0; i < 3; i++) {
                input_times[i] = input_times[i + 1];
                input_values[i] = input_values[i + 1];
            }
            input_times[3] += INPUT_INTERVAL;
            input_values[3] = input_times[3]/2; //sin(input_times[3]);
        }

        double interpolated = neville(next_output_time, input_times, input_values, 4);
        printf("Neville Output @ %.2f s: %.5f\n", next_output_time, interpolated);

        next_output_time += OUTPUT_INTERVAL;
    }

    return 0;
}
#endif
#if TAYLOR_1
#include <stdio.h>
#include <math.h>

#define INPUT_INTERVAL 1.0
#define OUTPUT_INTERVAL 0.1

// Linear interpolation between two points
double linear_interpolate(double t, double t0, double v0, double t1, double v1) {
    return v0 + ((v1 - v0) * (t - t0)) / (t1 - t0);
}

int main() {
    double simulation_duration = 10.0; // simulate for 10 seconds
    double next_input_time = 0.0;
    double next_output_time = 0.0;

    double current_input_value, next_input_value;

    // Initial input values
    current_input_value = next_input_time/2; //sin(next_input_time);
    next_input_time += INPUT_INTERVAL;
    next_input_value = next_input_time/2; //sin(next_input_time);

    while (next_output_time <= simulation_duration) {
        // Output interpolated samples at every 100ms
        if (next_output_time >= next_input_time) {
            // Move to next interval
            current_input_value = next_input_value;
            next_input_time += INPUT_INTERVAL;
            next_input_value = next_input_time/2; //sin(next_input_time);
        }

        // Linear interpolation
        double interpolated_value = linear_interpolate(
            next_output_time,
            next_input_time - INPUT_INTERVAL, current_input_value,
            next_input_time, next_input_value
        );

        //printf("Output @ %.1f s: %.4f\n", next_output_time, interpolated_value);
        printf("%.2f %.2f %.4f\n", next_input_time, next_output_time, interpolated_value);

        next_output_time += OUTPUT_INTERVAL;
    }

    return 0;
}
#endif

#if LAGRANGE

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_POLY_DEGREE 2
#define BUFFER_SIZE (MAX_POLY_DEGREE+1)  // Must be greater than MAX_POLY_DEGREE+1

typedef struct {
    float timestamp;
    float value;
} Sample;

typedef struct {
    Sample samples[BUFFER_SIZE];
    int count;
} Buffer;

// Add a new sample to the buffer (ring buffer logic)
void add_sample(Buffer *buf, float timestamp, float value) {
    if (buf->count < BUFFER_SIZE) {
        buf->samples[buf->count++] = (Sample){timestamp, value};
    } else {
        // Shift samples left to make space
        for (int i = 1; i < BUFFER_SIZE; i++)
            buf->samples[i - 1] = buf->samples[i];
        buf->samples[BUFFER_SIZE - 1] = (Sample){timestamp, value};
    }
}

// Perform Lagrange interpolation
float lagrange_interpolate(Buffer *buf, float t, int degree) {

    if (buf->count < degree + 1)
        return NAN;  // Not enough data

    // Find the 'degree+1' closest samples to 't'
    Sample closest[MAX_POLY_DEGREE + 1];

    // Create a temporary copy of samples
    Sample temp[BUFFER_SIZE];
    for (int i = 0; i < buf->count; i++)
        temp[i] = buf->samples[i];

    // Simple sorting by proximity to 't' (Bubble sort - acceptable for small BUFFER_SIZE)
    for (int i = 0; i < buf->count - 1; i++) {
        for (int j = i + 1; j < buf->count; j++) {
            if (fabs(temp[j].timestamp - t) < fabs(temp[i].timestamp - t)) {
                Sample swap = temp[i];
                temp[i] = temp[j];
                temp[j] = swap;
            }
        }
    }

    // Copy the closest (degree + 1) samples into 'closest'
    int points_to_copy = (buf->count > (degree + 1)) ? (degree + 1) : buf->count;
    for (int i = 0; i < points_to_copy; i++)
        closest[i] = temp[i];

    // Note: Ensure points_to_copy is always degree+1 when calling interpolation


    //// Find the 'degree+1' closest samples to 't'
    //Sample closest[degree + 1];

    // Simple sorting by proximity to t
    /*Sample temp[BUFFER_SIZE];*/
    for (int i = 0; i < buf->count; i++)
        temp[i] = buf->samples[i];

    for (int i = 0; i < buf->count - 1; i++) {
        for (int j = i + 1; j < buf->count; j++) {
            if (fabs(temp[j].timestamp - t) < fabs(temp[i].timestamp - t)) {
                Sample swap = temp[i];
                temp[i] = temp[j];
                temp[j] = swap;
            }
        }
    }

    for (int i = 0; i <= degree; i++)
        closest[i] = temp[i];

    // Compute Lagrange polynomial
    float result = 0.0;
    for (int i = 0; i <= degree; i++) {
        float term = closest[i].value;
        for (int j = 0; j <= degree; j++) {
            if (i != j) {
                term *= (t - closest[j].timestamp) / (closest[i].timestamp - closest[j].timestamp);
            }
        }
        result += term;
    }
    return result;
}

// Example usage
int main() {
    Buffer buffer = {.count = 0};
#define INPUT_INTERVAL 0.05
#define OUTPUT_INTERVAL 0.04
    // Simulate input samples (irregular intervals)
    float input_time = 0.0;
    float simulation_end = 30.0;
    float next_input_time = 0.0;

    // Output configuration
    float output_interval = OUTPUT_INTERVAL;  // 100ms
    float next_output_time = 0.0;
    float jitter, time_inc;

    srand(42);  // For reproducibility

    while (input_time <= simulation_end) {
        // Simulate jittery input every ~1-11s
        if (input_time >= next_input_time) {
            jitter = 0; //((float)rand() / RAND_MAX) * (INPUT_INTERVAL/2); // jitter

            float value = sin(2*3.141592654 * next_input_time/2);
            //float value = next_input_time/2; 
            add_sample(&buffer, next_input_time, value);

            //{   int i;
            //    float i1, i2, i0; 
            //    for (int i = BUFFER_SIZE-1; i > 0 ; i--) 
            //    {   i2 = (buffer.samples[i]).timestamp;
            //        i1 = (buffer.samples[i-1]).timestamp;
            //        i0 = i2-i1;
            //        (buffer.samples[i]).timestamp = i0;
            //    }
            //    buffer.samples[0].timestamp = 0;
            //}
            time_inc = (float)INPUT_INTERVAL + jitter;
            next_input_time += time_inc;
        }

        // Output at fixed intervals
        while (next_output_time <= input_time) {
            float interpolated = lagrange_interpolate(&buffer, next_output_time, MAX_POLY_DEGREE);
            if (!isnan(interpolated)) {
                printf("%4.2f %4.2f %4.2f %7.4f  S0 %6.3f[%6.3f] S1 %6.3f[%6.3f] S2 %6.3f[%6.3f]  interpolated = %6.3f\n", 
                    time_inc, input_time, next_input_time, next_output_time, 
                    buffer.samples[0].value, buffer.samples[0].timestamp, 
                    buffer.samples[1].value, buffer.samples[1].timestamp, 
                    buffer.samples[2].value, buffer.samples[2].timestamp, interpolated
                    );
            } else {
                printf("inc input_t next_in next_ou \n Output @ %.2f s: insufficient data\n", next_output_time);
            }
            next_output_time += output_interval;
        }

        // Advance simulation time
        input_time += time_inc; //INPUT_INTERVAL;  // Step increment for simulation accuracy  TIME CHUNKS
    }

    return 0;
}
#endif
#if NEVILLE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_POLY_DEGREE 3
#define BUFFER_SIZE 10  // must be greater than MAX_POLY_DEGREE + 1

typedef struct {
    double timestamp;
    double value;
} Sample;

typedef struct {
    Sample samples[BUFFER_SIZE];
    int count;
} Buffer;

// Add sample into buffer (ring buffer behavior)
void add_sample(Buffer *buf, double timestamp, double value) {
    if (buf->count < BUFFER_SIZE) {
        buf->samples[buf->count++] = (Sample){timestamp, value};
    } else {
        // Shift samples left to make space
        for (int i = 1; i < BUFFER_SIZE; i++)
            buf->samples[i - 1] = buf->samples[i];
        buf->samples[BUFFER_SIZE - 1] = (Sample){timestamp, value};
    }
}

// Neville's algorithm for polynomial interpolation
double neville_interpolate(Sample points[], int n, double t) {
    double Q[MAX_POLY_DEGREE + 1][MAX_POLY_DEGREE + 1];

    // Initialize Q[i][0]
    for (int i = 0; i < n; i++)
        Q[i][0] = points[i].value;

    // Neville's iterative computation
    for (int j = 1; j < n; j++) {
        for (int i = 0; i < n - j; i++) {
            Q[i][j] = ((t - points[i + j].timestamp) * Q[i][j - 1] +
                       (points[i].timestamp - t) * Q[i + 1][j - 1]) /
                      (points[i].timestamp - points[i + j].timestamp);
        }
    }

    return Q[0][n - 1];
}

// Select closest points to interpolation time 't'
int select_closest_samples(Buffer *buf, double t, int degree, Sample selected[]) {
    if (buf->count < degree + 1)
        return 0;  // insufficient data

    Sample temp[BUFFER_SIZE];
    for (int i = 0; i < buf->count; i++)
        temp[i] = buf->samples[i];

    // Simple bubble-sort by closeness to 't'
    for (int i = 0; i < buf->count - 1; i++) {
        for (int j = i + 1; j < buf->count; j++) {
            if (fabs(temp[j].timestamp - t) < fabs(temp[i].timestamp - t)) {
                Sample swap = temp[i];
                temp[i] = temp[j];
                temp[j] = swap;
            }
        }
    }

    // Copy the closest (degree + 1) samples
    for (int i = 0; i <= degree; i++)
        selected[i] = temp[i];

    return 1;
}

// Example usage
int main() {
    Buffer buffer = {.count = 0};

    // Simulated irregular input samples
    double input_time = 0.0;
    double simulation_end = 30.0;
    double next_input_time = 0.0;

    // Output configuration
    double output_interval = 0.1;  // 100 ms
    double next_output_time = 0.0;

    srand(42);  // reproducibility

    while (input_time <= simulation_end) {
        // Generate jittery input every ~1-11 seconds
        if (input_time >= next_input_time) {
            double jitter = 1.0 + (rand() % 1000) / 100.0;  // 1 to 11 sec jitter
            double value = next_input_time/ 2; //sin(next_input_time);  // example function
            add_sample(&buffer, next_input_time, value);
            next_input_time += jitter;
        }

        // Output at fixed intervals
        while (next_output_time <= input_time) {
            Sample selected[MAX_POLY_DEGREE + 1];
            if (select_closest_samples(&buffer, next_output_time, MAX_POLY_DEGREE, selected)) {
                double interpolated = neville_interpolate(selected, MAX_POLY_DEGREE + 1, next_output_time);
                //printf("Output @ %.2f s: %.4f\n", next_output_time, interpolated);
                printf("%.2f %.2f %.4f\n", next_input_time, next_output_time, interpolated);
            } else {
                //printf("Output @ %.2f s: insufficient data\n", next_output_time);
            }
            next_output_time += output_interval;
        }

        // Advance simulation time
        input_time += 0.05;  // simulation step increment
    }

    return 0;
}

#endif
