// 定数バッファ(CPU側からの値受け取り場)
cbuffer global
{
    matrix gWVP; // 変換行列
};

struct VS_IN
{
    float4 Pos : POSITION;
    float4 Color : COLOR;
};

struct VS_OUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
};

// 頂点シェーダ
VS_OUT VS(VS_IN input)
{
    VS_OUT vout;
    vout.Pos = input.Pos;
    vout.Color = input.Color;
    return vout;
}
 
// ピクセルシェーダ
float4 PS(VS_OUT input) : SV_Target
{
    return input.Color;
}