// �萔�o�b�t�@(CPU������̒l�󂯎���)
cbuffer global
{
    matrix gWVP; // �ϊ��s��
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

// ���_�V�F�[�_
VS_OUT VS(VS_IN input)
{
    VS_OUT vout;
    vout.Pos = input.Pos;
    vout.Color = input.Color;
    return vout;
}
 
// �s�N�Z���V�F�[�_
float4 PS(VS_OUT input) : SV_Target
{
    return input.Color;
}