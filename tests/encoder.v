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

    reg pinEncoderFPrev;

	always @(posedge clk) begin
        if (pinEncoderFPrev!=pinEncoderF && pinEncoderF==1'b1) begin
            encoderCounter<=1;
        end
        pinEncoderFPrev<=pinEncoderF;
        if (!resetn) begin
			encoderCount <= 0;
            encoderCounter <= 0;
		end else begin
            if (writeEncoder==1'b1) begin
                encoderCount<=encoderData;
            end
            if (encoderCounter>=1 && encoderCounter[3]!=1'b1 && pinEncoderF==1'b1) begin
                encoderCounter<=encoderCounter+1;
            end else if (encoderCounter[3]==1'b1) begin
                if (pinEncoderB==1'b0) begin
                    encoderCount<=encoderCount+1;
                end else begin
                    encoderCount<=encoderCount-1;
                end
                encoderCounter<=0;
            end else begin
            encoderCounter<=0;
            end
		end
		
	end


endmodule
