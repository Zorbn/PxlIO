#include "GLRenderer.hpp"

#include <stdexcept>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>

const char *vertexShaderSource =
	"#version 300 es\n"

	"precision highp float;\n"

	"layout (location = 0) in vec3 aVertexPos;\n"
	"layout (location = 1) in vec2 aTexCoord;\n"
	"layout (location = 2) in vec4 aColor;\n"

	"out vec2 TexCoord;\n"
	"out vec4 Color;\n"

	"uniform mat4 Proj;\n"

	"void main()\n"
	"{\n"
	"   vec4 pos = Proj * vec4(aVertexPos, 1.0);\n"

	"   gl_Position = pos;\n"
	"	TexCoord = aTexCoord;\n"
	"	Color = aColor;\n"
	"}\0";

const char *fragmentShaderSource =
	"#version 300 es\n"

	"precision highp float;\n"
	"precision mediump sampler2D;\n"

	"out vec4 FragColor;\n"

	"in vec2 TexCoord;\n"
	"in vec4 Color;\n"

	"uniform sampler2D Texture;\n"

	"void main()\n"
	"{\n"
	"   vec4 texColor = texture(Texture, TexCoord);\n"
	"	texColor = vec4(mix(texColor.rgb, Color.rgb, Color.a), texColor.a);\n"

	"   // Don't render transparent pixels.\n"
	"   if (texColor.a < 1.0)\n"
	"   {\n"
	"       discard;\n"
	"   }\n"

	"   FragColor = texColor;\n"
	"}\0";

const char *screenVertexShaderSource =
	"#version 300 es\n"

	"precision highp float;\n"

	"layout (location = 0) in vec3 aPos;\n"

	"out vec2 TexCoord;\n"

	"uniform mat4 Proj;"
	"uniform vec2 ViewSize;"
	"uniform vec2 Offset;"

	"void main()\n"
	"{\n"
	"	gl_Position = Proj * vec4(Offset + aPos.xy * ViewSize, 0.0, 1.0);\n"
	"	TexCoord = aPos.xy;\n"
	"}\0";

const char *screenFragmentShaderSource =
	"#version 300 es\n"

	"precision highp float;\n"
	"precision mediump sampler2D;\n"

	"out vec4 FragColor;\n"

	"in vec2 TexCoord;\n"

	"uniform sampler2D Tex;\n"

	"void main()\n"
	"{\n"
	"	vec4 texColor = texture(Tex, TexCoord);\n"
	"	FragColor = texColor;\n"
	"}\0";

const int32_t maxShaderErrorLen = 512;

const std::vector<float> screenVertices = {
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Bottom left
	1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // Bottom right
	1.0f, 1.0f, 0.0f, 1.0f, 1.0f, // Top right
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Bottom left
	1.0f, 1.0f, 0.0f, 1.0f, 1.0f, // Top right
	0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // Top left
};

GLRenderer::GLRenderer(const std::string &windowName, int32_t windowWidth, int32_t windowHeight,
					   int32_t viewWidth, int32_t viewHeight, bool enableVsync)
	: windowWidth(windowWidth), windowHeight(windowHeight),
	  viewWidth(viewWidth), viewHeight(viewHeight)
{

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		RUNTIME_ERROR("Failed to initialize SDL!");
	}

	if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
	{
		RUNTIME_ERROR("Failed to initialize SDL Image!");
	}

	window = SDL_CreateWindow(windowName.c_str(), SDL_WINDOWPOS_CENTERED,
							  SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight,
							  SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	if (!window)
	{
		RUNTIME_ERROR("Failed to create a window!");
	}

	SDL_GL_LoadLibrary(nullptr);

	SDL_GLContext context = SDL_GL_CreateContext(window);
	if (!context)
	{
		RUNTIME_ERROR("Failed to create GL context!");
	}

#ifndef EMSCRIPTEN
	SDL_GL_MakeCurrent(window, context);
	if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
	{
		RUNTIME_ERROR("Failed to load GL!");
	}
#endif

	SDL_GL_SetSwapInterval(enableVsync ? 1 : 0);

	// Sprite shader:
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
	glCompileShader(vertexShader);
	CheckShaderCompileError(vertexShader);

	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
	glCompileShader(fragmentShader);
	CheckShaderCompileError(fragmentShader);

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	CheckShaderLinkError(shaderProgram);

	glUseProgram(shaderProgram);
	projLocation = glGetUniformLocation(shaderProgram, "Proj");

	// Screen shader:
	screenVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(screenVertexShader, 1, &screenVertexShaderSource, nullptr);
	glCompileShader(screenVertexShader);
	CheckShaderCompileError(screenVertexShader);

	screenFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(screenFragmentShader, 1, &screenFragmentShaderSource, nullptr);
	glCompileShader(screenFragmentShader);
	CheckShaderCompileError(screenFragmentShader);

	screenShaderProgram = glCreateProgram();
	glAttachShader(screenShaderProgram, screenVertexShader);
	glAttachShader(screenShaderProgram, screenFragmentShader);
	glLinkProgram(screenShaderProgram);
	CheckShaderLinkError(screenShaderProgram);

	// Screen model:
	glGenVertexArrays(1, &screenVao);
	glBindVertexArray(screenVao);

	glGenBuffers(1, &screenVbo);
	glBindBuffer(GL_ARRAY_BUFFER, screenVbo);
	glBufferData(GL_ARRAY_BUFFER, screenVertices.size() * sizeof(float), &screenVertices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));

	glGenFramebuffers(1, &screenFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, screenFramebuffer);

	glGenTextures(1, &screenTexture);
	glBindTexture(GL_TEXTURE_2D, screenTexture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewWidth, viewHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glGenRenderbuffers(1, &screenDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, screenDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, viewWidth, viewHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, screenDepth);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);

	screenTexLocation = glGetUniformLocation(screenShaderProgram, "Tex");
	glUniform1i(screenTexLocation, 0);
	screenProjLocation = glGetUniformLocation(screenShaderProgram, "Proj");
	screenViewSizeLocation = glGetUniformLocation(screenShaderProgram, "ViewSize");
	screenOffsetLocation = glGetUniformLocation(screenShaderProgram, "Offset");

	// Sprite model:
	uint32_t spriteVao;
	glGenVertexArrays(1, &spriteVao);
	glBindVertexArray(spriteVao);

	uint32_t spriteVbo;
	glGenBuffers(1, &spriteVbo);
	glBindBuffer(GL_ARRAY_BUFFER, spriteVbo);
	glBufferData(GL_ARRAY_BUFFER, spriteVertices.size() * sizeof(float), &spriteVertices[0], GL_STATIC_DRAW);

	uint32_t spriteEbo;
	glGenBuffers(1, &spriteEbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, spriteEbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, spriteIndices.size() * sizeof(uint32_t), &spriteIndices[0],
				 GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(5 * sizeof(float)));

	spriteModel = GLModel{
		spriteVao,
		spriteVbo,
		spriteEbo,
		spriteIndices.size(),
	};

	glEnable(GL_DEPTH_TEST);
	// Face culling is disabled to allow flipping sprites: glEnable(GL_CULL_FACE)

	ResizeWindow(windowWidth, windowHeight);
}

GLRenderer::~GLRenderer()
{
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteProgram(shaderProgram);

	SDL_Quit();
}

void GLRenderer::ResizeWindow(int32_t windowWidth, int32_t windowHeight)
{
	this->windowWidth = windowWidth;
	this->windowHeight = windowHeight;

	glUseProgram(screenShaderProgram);
	glm::mat4 screenProj = glm::ortho<float>(0.0f, static_cast<float>(windowWidth),
											 0.0f, static_cast<float>(windowHeight), 0.0f, 1.0f);
	glUniformMatrix4fv(screenProjLocation, 1, GL_FALSE, glm::value_ptr(screenProj));

	ViewTransform viewTransform = Renderer::CalcViewTransform(windowWidth, windowHeight,
        viewWidth, viewHeight);

	glUniform2f(screenViewSizeLocation, viewTransform.scaledViewWidth, viewTransform.scaledViewHeight);
	glUniform2f(screenOffsetLocation, viewTransform.offsetX, viewTransform.offsetY);
}

SDL_Window *GLRenderer::GetWindowPtr()
{
	return window;
}

void GLRenderer::SetBackgroundColor(float r, float g, float b)
{
	backgroundR = r;
	backgroundG = g;
	backgroundB = b;
}

void GLRenderer::SetScreenBackgroundColor(float r, float g, float b)
{
	screenBackgroundR = r;
	screenBackgroundG = g;
	screenBackgroundB = b;
}

void GLRenderer::BeginDrawing()
{
	glUseProgram(shaderProgram);

	float viewWidthFloat = static_cast<float>(viewWidth);
	float viewHeightFloat = static_cast<float>(viewHeight);
	glm::mat4 proj = glm::ortho<float>(0.0, viewWidthFloat, 0.0f, viewHeightFloat, zNear, zFar);
	glUniformMatrix4fv(projLocation, 1, GL_FALSE, glm::value_ptr(proj));

	glBindFramebuffer(GL_FRAMEBUFFER, screenFramebuffer);
	glViewport(0, 0, viewWidth, viewHeight);
	glClearColor(backgroundR, backgroundG, backgroundB, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GLRenderer::EndDrawing()
{
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glViewport(0, 0, windowWidth, windowHeight);
	glUseProgram(screenShaderProgram);
	glClearColor(screenBackgroundR, screenBackgroundG, screenBackgroundB, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D, screenTexture);
	glBindVertexArray(screenVao);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	SDL_GL_SwapWindow(window);

#ifdef EMSCRIPTEN
	emscripten_sleep(0);
#endif
}

SpriteBatch GLRenderer::CreateSpriteBatch(const std::string &texturePath, uint32_t maxSprites, bool smooth)
{
	SDL_Surface *surface = LoadSurface(texturePath);

	auto data = reinterpret_cast<uint8_t *>(surface->pixels);
	auto textureWidth = surface->w;
	auto textureHeight = surface->h;
	auto format = GL_RGBA;

	GLTexture texture;
	glGenTextures(1, &texture.id);
	glBindTexture(GL_TEXTURE_2D, texture.id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	uint32_t minFilter = smooth ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST;
	uint32_t magFilter = smooth ? GL_LINEAR : GL_NEAREST;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

	glTexImage2D(GL_TEXTURE_2D, 0, format, textureWidth, textureHeight, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	SDL_FreeSurface(surface);

	auto spriteBatch = SpriteBatch(textureWidth, textureHeight, maxSprites);

	spriteBatchTextures.insert(std::make_pair(spriteBatch.GetId(), texture));

	return spriteBatch;
}

void GLRenderer::DestroySpriteBatch(SpriteBatch &spriteBatch)
{
	if (spriteBatchTextures.find(spriteBatch.GetId()) == spriteBatchTextures.end())
	{
		return;
	}

	auto &textureId = spriteBatchTextures.at(spriteBatch.GetId()).id;

	glDeleteTextures(1, &textureId);

	spriteBatchTextures.erase(spriteBatch.GetId());
}

void GLRenderer::DrawSpriteBatch(SpriteBatch &spriteBatch)
{
	if (spriteBatchTextures.find(spriteBatch.GetId()) == spriteBatchTextures.end())
	{
		return;
	}

	auto &textureId = spriteBatchTextures.at(spriteBatch.GetId()).id;

	glBindBuffer(GL_ARRAY_BUFFER, spriteModel.vbo);
	glBufferData(GL_ARRAY_BUFFER, spriteBatch.GetSpriteCount() * vertexValuesPerSprite * sizeof(float), &spriteBatch.GetVertices()[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, spriteModel.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, spriteBatch.GetSpriteCount() * indicesPerSprite * sizeof(uint32_t), &spriteBatch.GetIndices()[0],
				 GL_STATIC_DRAW);

	glUseProgram(shaderProgram);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glBindVertexArray(spriteModel.vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, spriteModel.ebo);

	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(spriteBatch.GetIndices().size()),
				   GL_UNSIGNED_INT, 0);
}

void GLRenderer::CheckShaderCompileError(uint32_t shader)
{
	int32_t success;

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		char infoLog[maxShaderErrorLen];
		glGetShaderInfoLog(shader, maxShaderErrorLen, nullptr, infoLog);
		RUNTIME_ERROR(infoLog);
	}
}

void GLRenderer::CheckShaderLinkError(uint32_t program)
{
	int32_t success;

	glGetProgramiv(program, GL_LINK_STATUS, &success);

	if (!success)
	{
		char infoLog[maxShaderErrorLen];
		glGetProgramInfoLog(program, maxShaderErrorLen, nullptr, infoLog);
		RUNTIME_ERROR(infoLog);
	}
}
