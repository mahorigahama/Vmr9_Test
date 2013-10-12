texture __tex0;
uint2 __tex0_size;

sampler sampler0=sampler_state
{
	Texture=<__tex0>;
	MinFilter=POINT;
	MagFilter=POINT;
	/*
	MinFilter=Linear;
	MagFilter=Linear;
	*/
	MipFilter=None;
	AddressU=Mirror;
	AddressV=Mirror;
};

////////////////////////////////////////////////////////////////////////////
// lanzcos レンダリング

struct VSOUT_LANZCOS
{
	float4 Pos  : POSITION;
	float2 Tex0 : TEXCOORD0;
	float2 Tex1 : TEXCOORD1;
	float2 Tex2 : TEXCOORD2;
	float2 Tex3 : TEXCOORD3;
	float2 Tex4 : TEXCOORD4;
	float2 Tex5 : TEXCOORD5;
	float2 Tex6 : TEXCOORD6;
	float2 Tex7 : TEXCOORD7;
};

VSOUT_LANZCOS VS_Lanzcos(float4 Pos : POSITION, float2 Tex : TEXCOORD)
{
	VSOUT_LANZCOS Out;
	Out.Pos =Pos;
	// Out.Tex0=Tex;
	Out.Tex0=Tex+float2(-1, -1)/__tex0_size;
	Out.Tex1=Tex+float2(-1,  0)/__tex0_size;
	Out.Tex2=Tex+float2(-1,  1)/__tex0_size;
	Out.Tex3=Tex+float2(-1,  2)/__tex0_size;
	Out.Tex4=Tex+float2( 0, -1)/__tex0_size;
	Out.Tex5=Tex;// +float2( 0,  0)/__tex0_size;
	Out.Tex6=Tex+float2( 0,  1)/__tex0_size;
	Out.Tex7=Tex+float2( 0,  2)/__tex0_size;
	return Out;
}

float4 PS_Lanzcos(VSOUT_LANZCOS input) : COLOR0
{
	float4 result=0;
		// 16テクセルをサンプリング。左上が col00, 右下が col33
		const float4 col00=tex2D(sampler0, input.Tex0);
	const float4 col01=tex2D(sampler0, input.Tex1);
	const float4 col02=tex2D(sampler0, input.Tex2);
	const float4 col03=tex2D(sampler0, input.Tex3);
	const float4 col10=tex2D(sampler0, input.Tex4);
	const float4 col11=tex2D(sampler0, input.Tex5);
	const float4 col12=tex2D(sampler0, input.Tex6);
	const float4 col13=tex2D(sampler0, input.Tex7);
	const float2 add  =float2(2.0f/__tex0_size.x, 0);
	const float4 col20=tex2D(sampler0, input.Tex0+add);
	const float4 col21=tex2D(sampler0, input.Tex1+add);
	const float4 col22=tex2D(sampler0, input.Tex2+add);
	const float4 col23=tex2D(sampler0, input.Tex3+add);
	const float4 col30=tex2D(sampler0, input.Tex4+add);
	const float4 col31=tex2D(sampler0, input.Tex5+add);
	const float4 col32=tex2D(sampler0, input.Tex6+add);
	const float4 col33=tex2D(sampler0, input.Tex7+add);
	// input.Tex5 が参照しているテクセルの左上端までの距離を計算する。
	float2 uv_d=frac(input.Tex5*__tex0_size)-0.5;
		// a == 2, asin(px)sin(px/a)/(px*px)
		const float minpd=0.000001;
	const float a=2.0;
	const float pi=3.1415926;
	float pd;
	float4 wx, wy;
	pd=max(minpd, abs(pi*(uv_d.x+1))); wx.x=a*sin(pd)*sin(pd/a)/(pd*pd);
	pd=max(minpd, abs(pi*(uv_d.x+0))); wx.y=a*sin(pd)*sin(pd/a)/(pd*pd);
	pd=max(minpd, abs(pi*(uv_d.x-1))); wx.z=a*sin(pd)*sin(pd/a)/(pd*pd);
	pd=max(minpd, abs(pi*(uv_d.x-2))); wx.w=a*sin(pd)*sin(pd/a)/(pd*pd);
	pd=max(minpd, abs(pi*(uv_d.y+1))); wy.x=a*sin(pd)*sin(pd/a)/(pd*pd);
	pd=max(minpd, abs(pi*(uv_d.y+0))); wy.y=a*sin(pd)*sin(pd/a)/(pd*pd);
	pd=max(minpd, abs(pi*(uv_d.y-1))); wy.z=a*sin(pd)*sin(pd/a)/(pd*pd);
	pd=max(minpd, abs(pi*(uv_d.y-2))); wy.w=a*sin(pd)*sin(pd/a)/(pd*pd);
	const float4 col_r0=wx.x*col00+wx.y*col10+wx.z*col20+wx.w*col30;
	const float4 col_r1=wx.x*col01+wx.y*col11+wx.z*col21+wx.w*col31;
	const float4 col_r2=wx.x*col02+wx.y*col12+wx.z*col22+wx.w*col32;
	const float4 col_r3=wx.x*col03+wx.y*col13+wx.z*col23+wx.w*col33;
	result=col_r0*wy.x+col_r1*wy.y+col_r2*wy.z+col_r3*wy.w;
	const float w_sum=
		wy.x*(wx.x+wx.y+wx.z+wx.w)
		+wy.y*(wx.x+wx.y+wx.z+wx.w)
		+wy.z*(wx.x+wx.y+wx.z+wx.w)
		+wy.w*(wx.x+wx.y+wx.z+wx.w);
	result/=w_sum;
	return result;
}

////////////////////////////////////////////////////////////////////////////
// シンプルなレンダリング
struct VSOUT_SIMPLE
{
	float4 Pos : POSITION;
	float2 Tex : TEXCOORD0;
};

VSOUT_SIMPLE VS_Simple(float4 Pos : POSITION, float2 Tex : TEXCOORD0)
{
	VSOUT_SIMPLE result;
	result.Pos=Pos;
	result.Tex=Tex;
	return result;
}

float4 PS_Simple(VSOUT_SIMPLE In) : COLOR0
{
	return tex2D(sampler0, In.Tex);
}

technique lanzcos
{
	pass p0
	{
		SrcBlend=One;
		DestBlend=Zero;
		CullMode=None;
		ShadeMode=Flat;
		ZEnable=False;
		AlphaTestEnable=False;
		VertexShader=compile vs_3_0 VS_Lanzcos();
		PixelShader =compile ps_3_0 PS_Lanzcos();
	}
}

technique simple
{
	pass p0
	{
		SrcBlend=One;
		DestBlend=Zero;
		CullMode=None;
		ShadeMode=Flat;
		ZEnable=False;
		AlphaTestEnable=False;
		VertexShader=compile vs_3_0 VS_Simple();
		PixelShader =compile ps_3_0 PS_Simple();
	}
}
