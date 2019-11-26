/*
 * Encoder Module
 */

module encoder (
	input clk,
    input resetn,
    input writeEncoder,
	input [31:0] encoderData,
    input pinEncoderF,
    input pinEncoderB,
	output [31:0]  encoderValue,
);
	reg [31:0] encoderCount=0;
    reg [3:0] encoderCounter=0;
    assign encoderValue = encoderCount;

    always @(*) begin

    pinEncoderF

	always @(posedge clk) begin
        if (!resetn) begin
			encoderCount <= 0;
            encoderCounter <= 0;
		end else begin
            if (writeEncoder==1'b1) begin
                encoderValue<=encoderData
            end
            if (encoderCounter>=1 && encoderCounter[3]!=1b'1 && pinEncoderF==1b'1) begin
                encoderCounter<=encoderCounter+1;
            end
            if (encoderCounter[3]==1b'1) begin
                if (pinEncoderB==1'b0) begin
                    encoderCount<=encoderCount+1;
                else
                    encoderCount<=encoderCount-1;
                end
            encoderCounter<=0;
            else begin
            encoderCounter<=0;
            end
		end
		
	end

    always @(posedge pinEncoderF) begin
    encoderCounter<=1;
    end



endmodule
