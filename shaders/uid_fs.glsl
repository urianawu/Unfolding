#version 330
uniform usamplerBuffer id_tex;
uniform int mode = 0;

out vec4 frag_color;

void main()
{
	int uid;
	switch (mode)
	{
	case 0://background
	{
		frag_color = vec4(0.f, 0.f, 0.f, 0.f);
		break;
	}
	case 1://vertex
	{
		uid = gl_PrimitiveID;
		frag_color = vec4(float(uid >> 16) / 255.0f,
			float((uid >> 8) & 0xFF) / 255.0f,
			float(uid & 0xFF) / 255.0f, 1.0f);
		break;
	}
	case 2://edge or face
	{
		uid = int(texelFetch(id_tex, gl_PrimitiveID).r);
		frag_color = vec4(float(uid >> 16) / 255.0f,
			float((uid >> 8) & 0xFF) / 255.0f,
			float(uid & 0xFF) / 255.0f, 1.0f);
		break;
	}
	default: break;
	}
}