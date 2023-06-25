module polynomial_pipe_rtl( clock, reset, ce, x, coef, coef_valid, result, valid_out, coef_reset, coef_ready, coef_last );
parameter integer W = 32; //Width of the fixed-point (24:8) representation
parameter FXP_SHIFT = 8; //Scaling factor for fixed-point (24:8) representation
parameter PIPE_LATENCY = 7; // Input->output delay in clock cycles
input clock, reset, ce, coef_reset, coef_last;
input [W-1:0] x, coef; // Polynomial = ax^3 + bx^2 + cx + d
input coef_valid;
output reg [W-1:0] result;
output reg valid_out; //Valid data output flag
output coef_ready;

//Tabs of registers for connections between stages
reg signed [W-1:0] value_tab [0:5];
reg signed [W-1:0] x_tab [0:4];
reg signed [W-1:0] a_var;
reg signed [W-1:0] b_tab [0:1]; 
reg signed [W-1:0] c_tab [0:3];
reg signed [W-1:0] d_tab [0:5];
reg signed [W-1:0] coef_tab [0:3];      //Input shift register

reg unsigned [4:0] valid_cnt; //Counts pipeline delay
assign coef_ready = 1; //Shift register is always ready to write

always@(posedge clock)
begin
    if ( reset == 1'b1 )
        valid_cnt <= PIPE_LATENCY; //Setup latency counter
    else begin
        if( ( valid_cnt != 0 ) && ( ce == 1'b1 ) ) begin
            valid_cnt <= valid_cnt - 1; //Valid output data moves toward output
        end
    end
    if (coef_valid == 1'b1)                 //Shift register update
    begin
        coef_tab[3] <= coef_tab[2];         //Shift values
        coef_tab[2] <= coef_tab[1]; 
        coef_tab[1] <= coef_tab[0];
        coef_tab[0] <= coef;                //Write input data to shift register
    end
    valid_out = ( valid_cnt == 0 )? 1'b1 : 1'b0; //Set valid_out when counter counts up to PIPE_LATENCY
    if (ce == 1'b1)
    begin
        //Stage a0
        x_tab[0] <= x;
        a_var <= coef_tab[3];
        b_tab[0] <= coef_tab[2];
        c_tab[0] <= coef_tab[1];
        d_tab[0] <= coef_tab[0];
        //Stage a1
        value_tab[0] <= (x_tab[0] * a_var) >> FXP_SHIFT;            //Multiply then scale for fixed point format
        x_tab[1] <= x_tab[0];
        b_tab[1] <= b_tab[0];
        c_tab[1] <= c_tab[0];
        d_tab[1] <= d_tab[0];
        // Stage a2
        value_tab[1] <= value_tab[0] + b_tab[1];
        x_tab[2] <= x_tab[1];
        c_tab[2] <= c_tab[1];
        d_tab[2] <= d_tab[1];
        // Stage a3
        value_tab[2] <= (value_tab[1] * x_tab[2]) >> FXP_SHIFT;     //Multiply then scale for fixed point format
        x_tab[3] <= x_tab[2];
        c_tab[3] <= c_tab[2];
        d_tab[3] <= d_tab[2];
        // Stage a4
        value_tab[3] <= value_tab[2] + c_tab[3];
        x_tab[4] <= x_tab[3];
        d_tab[4] <= d_tab[3];
        // Stage a5
        value_tab[4] <= (value_tab[3] * x_tab[4]) >> FXP_SHIFT;     //Multiply then scale for fixed point format 
        d_tab[5] <= d_tab[4];
        // Stage a6
        value_tab[5] <= value_tab[4] + d_tab[5];
        // Stage a7
        result <= value_tab[5];
    end
end
endmodule