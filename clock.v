/*
 * Clock Module
 */

module clock (
	input clk,
    input resetn,
	output [31:0] clock_out,
);
	reg [31:0] counterI=0;

    reg [31:0] counterO=32'hffffffff;

    assign clock_out = counterO;

	always @(posedge clk) begin
        if (!resetn) begin
			counterI <= 0;
            counterO <= 32'hffffffff;
		end else begin
			counterI<= counterI+1;
            if (counterI[10] == 1) begin
                counterI <= 0;
                counterO<= counterO+1;
			end
		end
		
	end


endmodule
