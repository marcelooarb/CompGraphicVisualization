///////////////////////////////////////////////////////////////////////////////
// scenemanager.cpp
// ============
// manage the preparing and rendering of 3D scenes - textures, materials, lighting
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 * SceneManager()
 ***********************************************************/
SceneManager::SceneManager(ShaderManager* pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();

	m_loadedTextures = 0;

	for (int i = 0; i < 16; i++)
	{
		m_textureIDs[i].ID = 0;
		m_textureIDs[i].tag = "";
	}
}

/***********************************************************
 * ~SceneManager()
 ***********************************************************/
SceneManager::~SceneManager()
{
	DestroyGLTextures();

	m_pShaderManager = NULL;

	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

/***********************************************************
 * CreateGLTexture()
 ***********************************************************/
bool SceneManager::CreateGLTexture(
	const char* filename,
	std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	stbi_set_flip_vertically_on_load(true);

	unsigned char* image =
		stbi_load(
			filename,
			&width,
			&height,
			&colorChannels,
			0);

	if (image)
	{
		std::cout
			<< "Successfully loaded image: "
			<< filename
			<< ", width: "
			<< width
			<< ", height: "
			<< height
			<< ", channels: "
			<< colorChannels
			<< std::endl;

		glGenTextures(
			1,
			&textureID);

		glBindTexture(
			GL_TEXTURE_2D,
			textureID);

		glTexParameteri(
			GL_TEXTURE_2D,
			GL_TEXTURE_WRAP_S,
			GL_REPEAT);

		glTexParameteri(
			GL_TEXTURE_2D,
			GL_TEXTURE_WRAP_T,
			GL_REPEAT);

		glTexParameteri(
			GL_TEXTURE_2D,
			GL_TEXTURE_MIN_FILTER,
			GL_LINEAR);

		glTexParameteri(
			GL_TEXTURE_2D,
			GL_TEXTURE_MAG_FILTER,
			GL_LINEAR);

		if (colorChannels == 3)
		{
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RGB8,
				width,
				height,
				0,
				GL_RGB,
				GL_UNSIGNED_BYTE,
				image);
		}
		else if (colorChannels == 4)
		{
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RGBA8,
				width,
				height,
				0,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				image);
		}
		else
		{
			stbi_image_free(image);

			glBindTexture(
				GL_TEXTURE_2D,
				0);

			glDeleteTextures(
				1,
				&textureID);

			return false;
		}

		glGenerateMipmap(
			GL_TEXTURE_2D);

		stbi_image_free(image);

		glBindTexture(
			GL_TEXTURE_2D,
			0);

		if (m_loadedTextures < 16)
		{
			m_textureIDs[m_loadedTextures].ID =
				textureID;

			m_textureIDs[m_loadedTextures].tag =
				tag;

			m_loadedTextures++;

			return true;
		}

		glDeleteTextures(
			1,
			&textureID);

		return false;
	}

	std::cout
		<< "Could not load image: "
		<< filename
		<< std::endl;

	return false;
}

/***********************************************************
 * BindGLTextures()
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glActiveTexture(
			GL_TEXTURE0 + i);

		glBindTexture(
			GL_TEXTURE_2D,
			m_textureIDs[i].ID);
	}
}

/***********************************************************
 * DestroyGLTextures()
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glDeleteTextures(
			1,
			&m_textureIDs[i].ID);
	}
}

/***********************************************************
 * FindTextureID()
 ***********************************************************/
int SceneManager::FindTextureID(
	std::string tag)
{
	int textureID = -1;

	for (int i = 0; i < m_loadedTextures; i++)
	{
		if (m_textureIDs[i].tag.compare(tag) == 0)
		{
			textureID =
				m_textureIDs[i].ID;

			break;
		}
	}

	return textureID;
}

/***********************************************************
 * FindTextureSlot()
 ***********************************************************/
int SceneManager::FindTextureSlot(
	std::string tag)
{
	int textureSlot = -1;

	for (int i = 0; i < m_loadedTextures; i++)
	{
		if (m_textureIDs[i].tag.compare(tag) == 0)
		{
			textureSlot = i;

			break;
		}
	}

	return textureSlot;
}

/***********************************************************
 * FindMaterial()
 ***********************************************************/
bool SceneManager::FindMaterial(
	std::string tag,
	OBJECT_MATERIAL& material)
{
	for (unsigned int i = 0;
		i < m_objectMaterials.size();
		i++)
	{
		if (m_objectMaterials[i].tag.compare(tag) == 0)
		{
			material =
				m_objectMaterials[i];

			return true;
		}
	}

	return false;
}

/***********************************************************
 * SetTransformations()
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	glm::mat4 scale =
		glm::scale(
			scaleXYZ);

	glm::mat4 rotationX =
		glm::rotate(
			glm::radians(XrotationDegrees),
			glm::vec3(
				1.0f,
				0.0f,
				0.0f));

	glm::mat4 rotationY =
		glm::rotate(
			glm::radians(YrotationDegrees),
			glm::vec3(
				0.0f,
				1.0f,
				0.0f));

	glm::mat4 rotationZ =
		glm::rotate(
			glm::radians(ZrotationDegrees),
			glm::vec3(
				0.0f,
				0.0f,
				1.0f));

	glm::mat4 translation =
		glm::translate(
			positionXYZ);

	glm::mat4 modelView =
		translation *
		rotationZ *
		rotationY *
		rotationX *
		scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(
			g_ModelName,
			modelView);
	}
}

/***********************************************************
 * SetShaderColor()
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	glm::vec4 currentColor(
		redColorValue,
		greenColorValue,
		blueColorValue,
		alphaValue);

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(
			g_UseTextureName,
			false);

		m_pShaderManager->setVec4Value(
			g_ColorValueName,
			currentColor);
	}
}

/***********************************************************
 * SetShaderTexture()
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		int textureSlot =
			FindTextureSlot(
				textureTag);

		if (textureSlot >= 0)
		{
			m_pShaderManager->setIntValue(
				g_UseTextureName,
				true);

			m_pShaderManager->setSampler2DValue(
				g_TextureValueName,
				textureSlot);
		}
		else
		{
			m_pShaderManager->setIntValue(
				g_UseTextureName,
				false);
		}
	}
}

/***********************************************************
 * SetTextureUVScale()
 ***********************************************************/
void SceneManager::SetTextureUVScale(
	float u,
	float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value(
			"UVscale",
			glm::vec2(
				u,
				v));
	}
}

/***********************************************************
 * SetShaderMaterial()
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	OBJECT_MATERIAL material;

	if (FindMaterial(
		materialTag,
		material))
	{
		m_pShaderManager->setVec3Value(
			"material.diffuseColor",
			material.diffuseColor);

		m_pShaderManager->setVec3Value(
			"material.specularColor",
			material.specularColor);

		m_pShaderManager->setFloatValue(
			"material.shininess",
			material.shininess);
	}
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/**************************************************************/

/***********************************************************
 * LoadSceneTextures()
 *
 * Loads all texture image files and binds them for use
 * when rendering the scene.
 ***********************************************************/
void SceneManager::LoadSceneTextures()
{
	bool bReturn = false;

	// Wooden desktop.
	bReturn =
		CreateGLTexture(
			"textures/wood_planks_seamless.jpg",
			"desk");

	if (bReturn == false)
	{
		CreateGLTexture(
			"../textures/wood_planks_seamless.jpg",
			"desk");
	}


	// Galvanized metal used for the laptop stand.
	bReturn =
		CreateGLTexture(
			"textures/metal_galvanized_seamless.jpg",
			"metal");

	if (bReturn == false)
	{
		CreateGLTexture(
			"../textures/metal_galvanized_seamless.jpg",
			"metal");
	}


	// Laptop keyboard surface.
	bReturn =
		CreateGLTexture(
			"textures/laptop_2.jpg",
			"laptopKeyboard");

	if (bReturn == false)
	{
		CreateGLTexture(
			"../textures/laptop_2.jpg",
			"laptopKeyboard");
	}


	// External desktop keyboard.
	bReturn =
		CreateGLTexture(
			"textures/keyboard_external.jpg",
			"externalKeyboard");

	if (bReturn == false)
	{
		CreateGLTexture(
			"../textures/keyboard_external.jpg",
			"externalKeyboard");
	}


	// Dark plastic used on the laptop and monitor.
	bReturn =
		CreateGLTexture(
			"textures/plastic_dark_seamless.jpg",
			"plastic");

	if (bReturn == false)
	{
		CreateGLTexture(
			"../textures/plastic_dark_seamless.jpg",
			"plastic");
	}


	// Wallpaper used on both displays.
	bReturn =
		CreateGLTexture(
			"textures/screen_wallpaper_2.jpg",
			"display");

	if (bReturn == false)
	{
		CreateGLTexture(
			"../textures/screen_wallpaper_2.jpg",
			"display");
	}


	// Mouse pad.
	bReturn =
		CreateGLTexture(
			"textures/leather_black_seamless.jpg",
			"mousepad");

	if (bReturn == false)
	{
		CreateGLTexture(
			"../textures/leather_black_seamless.jpg",
			"mousepad");
	}

	// Bind all loaded textures to texture units.
	BindGLTextures();
}

/***********************************************************
 * DefineObjectMaterials()
 ***********************************************************/
void SceneManager::DefineObjectMaterials()
{
	OBJECT_MATERIAL woodMaterial;

	woodMaterial.diffuseColor =
		glm::vec3(
			1.0f,
			1.0f,
			1.0f);

	woodMaterial.specularColor =
		glm::vec3(
			0.22f,
			0.22f,
			0.22f);

	woodMaterial.shininess =
		12.0f;

	woodMaterial.tag =
		"wood";

	m_objectMaterials.push_back(
		woodMaterial);


	OBJECT_MATERIAL metalMaterial;

	metalMaterial.diffuseColor =
		glm::vec3(
			1.0f,
			1.0f,
			1.0f);

	metalMaterial.specularColor =
		glm::vec3(
			0.75f,
			0.75f,
			0.75f);

	metalMaterial.shininess =
		56.0f;

	metalMaterial.tag =
		"metal";

	m_objectMaterials.push_back(
		metalMaterial);


	OBJECT_MATERIAL plasticMaterial;

	plasticMaterial.diffuseColor =
		glm::vec3(
			1.0f,
			1.0f,
			1.0f);

	plasticMaterial.specularColor =
		glm::vec3(
			0.25f,
			0.25f,
			0.25f);

	plasticMaterial.shininess =
		20.0f;

	plasticMaterial.tag =
		"plastic";

	m_objectMaterials.push_back(
		plasticMaterial);


	OBJECT_MATERIAL screenMaterial;

	screenMaterial.diffuseColor =
		glm::vec3(
			1.0f,
			1.0f,
			1.0f);

	screenMaterial.specularColor =
		glm::vec3(
			0.10f,
			0.10f,
			0.10f);

	screenMaterial.shininess =
		8.0f;

	screenMaterial.tag =
		"screen";

	m_objectMaterials.push_back(
		screenMaterial);
}

/***********************************************************
 * SetupSceneLights()
 ***********************************************************/
void SceneManager::SetupSceneLights()
{
	m_pShaderManager->setBoolValue(
		g_UseLightingName,
		true);

	m_pShaderManager->setBoolValue(
		"directionalLight.bActive",
		false);

	m_pShaderManager->setBoolValue(
		"spotLight.bActive",
		false);


	// Main white point light.
	m_pShaderManager->setVec3Value(
		"pointLights[0].position",
		5.0f,
		10.0f,
		8.0f);

	m_pShaderManager->setVec3Value(
		"pointLights[0].ambient",
		0.18f,
		0.18f,
		0.18f);

	m_pShaderManager->setVec3Value(
		"pointLights[0].diffuse",
		0.75f,
		0.75f,
		0.75f);

	m_pShaderManager->setVec3Value(
		"pointLights[0].specular",
		0.65f,
		0.65f,
		0.65f);

	m_pShaderManager->setBoolValue(
		"pointLights[0].bActive",
		true);


	// Softer blue fill light.
	m_pShaderManager->setVec3Value(
		"pointLights[1].position",
		-5.0f,
		7.0f,
		4.0f);

	m_pShaderManager->setVec3Value(
		"pointLights[1].ambient",
		0.02f,
		0.03f,
		0.05f);

	m_pShaderManager->setVec3Value(
		"pointLights[1].diffuse",
		0.08f,
		0.18f,
		0.35f);

	m_pShaderManager->setVec3Value(
		"pointLights[1].specular",
		0.10f,
		0.25f,
		0.50f);

	m_pShaderManager->setBoolValue(
		"pointLights[1].bActive",
		true);


	m_pShaderManager->setBoolValue(
		"pointLights[2].bActive",
		false);

	m_pShaderManager->setBoolValue(
		"pointLights[3].bActive",
		false);

	m_pShaderManager->setBoolValue(
		"pointLights[4].bActive",
		false);
}

/***********************************************************
 * PrepareScene()
 ***********************************************************/
void SceneManager::PrepareScene()
{
	// Load and bind all scene textures.
	LoadSceneTextures();

	// Define material properties used by the lighting model.
	DefineObjectMaterials();

	// Configure the scene lighting.
	SetupSceneLights();

	// Load the primitive meshes used to build the objects.
	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadSphereMesh();
}

/***********************************************************
 * RenderScene()
 ***********************************************************/
void SceneManager::RenderScene()
{
	glm::vec3 scaleXYZ;
	glm::vec3 positionXYZ;


	/****************************************************************/
	// DESK
	/****************************************************************/
	scaleXYZ =
		glm::vec3(
			20.0f,
			1.0f,
			10.0f);

	positionXYZ =
		glm::vec3(
			0.0f,
			0.0f,
			0.0f);

	SetTransformations(
		scaleXYZ,
		0.0f,
		0.0f,
		0.0f,
		positionXYZ);

	SetShaderTexture("desk");
	SetTextureUVScale(6.0f, 4.0f);
	SetShaderMaterial("wood");

	m_basicMeshes->DrawPlaneMesh();


	/****************************************************************/
	// LAPTOP STAND - LEFT LOWER BASE RAIL
	/****************************************************************/
	scaleXYZ =
		glm::vec3(
			0.28f,
			0.20f,
			3.80f);

	positionXYZ =
		glm::vec3(
			-4.50f,
			0.15f,
			0.0f);

	SetTransformations(
		scaleXYZ,
		0.0f,
		0.0f,
		0.0f,
		positionXYZ);

	SetShaderTexture("metal");
	SetTextureUVScale(4.0f, 1.0f);
	SetShaderMaterial("metal");

	m_basicMeshes->DrawBoxMesh();


	/****************************************************************/
	// LAPTOP STAND - RIGHT LOWER BASE RAIL
	/****************************************************************/
	positionXYZ =
		glm::vec3(
			-1.50f,
			0.15f,
			0.0f);

	SetTransformations(
		scaleXYZ,
		0.0f,
		0.0f,
		0.0f,
		positionXYZ);

	SetShaderTexture("metal");
	SetTextureUVScale(4.0f, 1.0f);
	SetShaderMaterial("metal");

	m_basicMeshes->DrawBoxMesh();


	/****************************************************************/
	// LAPTOP STAND - FRONT CROSSBAR
	/****************************************************************/
	scaleXYZ =
		glm::vec3(
			3.28f,
			0.20f,
			0.28f);

	positionXYZ =
		glm::vec3(
			-3.0f,
			0.15f,
			1.86f);

	SetTransformations(
		scaleXYZ,
		0.0f,
		0.0f,
		0.0f,
		positionXYZ);

	SetShaderTexture("metal");
	SetTextureUVScale(4.0f, 1.0f);
	SetShaderMaterial("metal");

	m_basicMeshes->DrawBoxMesh();


	/****************************************************************/
	// LAPTOP STAND - LEFT ANGLED UPPER RAIL
	/****************************************************************/
	scaleXYZ =
		glm::vec3(
			0.28f,
			0.20f,
			3.80f);

	positionXYZ =
		glm::vec3(
			-4.50f,
			1.58f,
			0.0f);

	SetTransformations(
		scaleXYZ,
		18.0f,
		0.0f,
		0.0f,
		positionXYZ);

	SetShaderTexture("metal");
	SetTextureUVScale(4.0f, 1.0f);
	SetShaderMaterial("metal");

	m_basicMeshes->DrawBoxMesh();


	/****************************************************************/
	// LAPTOP STAND - RIGHT ANGLED UPPER RAIL
	/****************************************************************/
	positionXYZ =
		glm::vec3(
			-1.50f,
			1.58f,
			0.0f);

	SetTransformations(
		scaleXYZ,
		18.0f,
		0.0f,
		0.0f,
		positionXYZ);

	SetShaderTexture("metal");
	SetTextureUVScale(4.0f, 1.0f);
	SetShaderMaterial("metal");

	m_basicMeshes->DrawBoxMesh();


	/****************************************************************/
	// LAPTOP STAND - LEFT REAR SUPPORT
	/****************************************************************/
	scaleXYZ =
		glm::vec3(
			0.28f,
			1.90f,
			0.28f);

	positionXYZ =
		glm::vec3(
			-4.50f,
			1.08f,
			-1.72f);

	SetTransformations(
		scaleXYZ,
		0.0f,
		0.0f,
		0.0f,
		positionXYZ);

	SetShaderTexture("metal");
	SetTextureUVScale(1.0f, 3.0f);
	SetShaderMaterial("metal");

	m_basicMeshes->DrawBoxMesh();


	/****************************************************************/
	// LAPTOP STAND - RIGHT REAR SUPPORT
	/****************************************************************/
	positionXYZ =
		glm::vec3(
			-1.50f,
			1.08f,
			-1.72f);

	SetTransformations(
		scaleXYZ,
		0.0f,
		0.0f,
		0.0f,
		positionXYZ);

	SetShaderTexture("metal");
	SetTextureUVScale(1.0f, 3.0f);
	SetShaderMaterial("metal");

	m_basicMeshes->DrawBoxMesh();


	/****************************************************************/
	// LAPTOP STAND - LEFT RETAINING LIP
	/****************************************************************/
	scaleXYZ =
		glm::vec3(
			0.28f,
			0.60f,
			0.28f);

	positionXYZ =
		glm::vec3(
			-4.50f,
			1.24f,
			1.78f);

	SetTransformations(
		scaleXYZ,
		0.0f,
		0.0f,
		0.0f,
		positionXYZ);

	SetShaderTexture("metal");
	SetTextureUVScale(1.0f, 2.0f);
	SetShaderMaterial("metal");

	m_basicMeshes->DrawBoxMesh();


	/****************************************************************/
	// LAPTOP STAND - RIGHT RETAINING LIP
	/****************************************************************/
	positionXYZ =
		glm::vec3(
			-1.50f,
			1.24f,
			1.78f);

	SetTransformations(
		scaleXYZ,
		0.0f,
		0.0f,
		0.0f,
		positionXYZ);

	SetShaderTexture("metal");
	SetTextureUVScale(1.0f, 2.0f);
	SetShaderMaterial("metal");

	m_basicMeshes->DrawBoxMesh();


	/****************************************************************/
	// LAPTOP BASE
	/****************************************************************/
	scaleXYZ =
		glm::vec3(
			3.35f,
			0.16f,
			3.25f);

	positionXYZ =
		glm::vec3(
			-3.0f,
			1.78f,
			-0.05f);

	SetTransformations(
		scaleXYZ,
		18.0f,
		0.0f,
		0.0f,
		positionXYZ);

	SetShaderMaterial("plastic");
	SetTextureUVScale(1.0f, 1.0f);

	SetShaderTexture("plastic");

	m_basicMeshes->DrawBoxMeshSide(
		ShapeMeshes::box_back);

	m_basicMeshes->DrawBoxMeshSide(
		ShapeMeshes::box_bottom);

	m_basicMeshes->DrawBoxMeshSide(
		ShapeMeshes::box_left);

	m_basicMeshes->DrawBoxMeshSide(
		ShapeMeshes::box_right);

	m_basicMeshes->DrawBoxMeshSide(
		ShapeMeshes::box_front);

	// Keyboard texture only on the top of the laptop.
	SetShaderTexture(
		"laptopKeyboard");

	m_basicMeshes->DrawBoxMeshSide(
		ShapeMeshes::box_top);


	/****************************************************************/
	// LAPTOP SCREEN HOUSING
	/****************************************************************/
	scaleXYZ =
		glm::vec3(
			3.35f,
			2.45f,
			0.16f);

	positionXYZ =
		glm::vec3(
			-3.0f,
			3.38f,
			-1.48f);

	SetTransformations(
		scaleXYZ,
		-10.0f,
		0.0f,
		0.0f,
		positionXYZ);

	SetShaderTexture("plastic");
	SetTextureUVScale(2.0f, 2.0f);
	SetShaderMaterial("plastic");

	m_basicMeshes->DrawBoxMesh();


	/****************************************************************/
	// LAPTOP DISPLAY
	/****************************************************************/
	scaleXYZ =
		glm::vec3(
			2.95f,
			2.05f,
			0.06f);

	positionXYZ =
		glm::vec3(
			-3.0f,
			3.38f,
			-1.37f);

	SetTransformations(
		scaleXYZ,
		-10.0f,
		0.0f,
		0.0f,
		positionXYZ);

	SetShaderTexture("display");
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("screen");

	m_basicMeshes->DrawBoxMesh();


	/****************************************************************/
	// LARGE MONITOR HOUSING
	/****************************************************************/
	scaleXYZ =
		glm::vec3(
			5.60f,
			3.15f,
			0.20f);

	positionXYZ =
		glm::vec3(
			1.35f,
			3.22f,
			-2.35f);

	SetTransformations(
		scaleXYZ,
		0.0f,
		-3.0f,
		0.0f,
		positionXYZ);

	SetShaderTexture("plastic");
	SetTextureUVScale(2.0f, 2.0f);
	SetShaderMaterial("plastic");

	m_basicMeshes->DrawBoxMesh();


	/****************************************************************/
	// LARGE MONITOR DISPLAY
	/****************************************************************/
	scaleXYZ =
		glm::vec3(
			5.18f,
			2.77f,
			0.06f);

	positionXYZ =
		glm::vec3(
			1.35f,
			3.22f,
			-2.18f);

	SetTransformations(
		scaleXYZ,
		0.0f,
		-3.0f,
		0.0f,
		positionXYZ);

	SetShaderTexture("display");
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("screen");

	m_basicMeshes->DrawBoxMesh();


	/****************************************************************/
	// MONITOR STAND - UPPER MOUNT
	/****************************************************************/
	scaleXYZ =
		glm::vec3(
			0.62f,
			0.32f,
			0.32f);

	positionXYZ =
		glm::vec3(
			1.35f,
			1.72f,
			-2.17f);

	SetTransformations(
		scaleXYZ,
		0.0f,
		0.0f,
		0.0f,
		positionXYZ);

	SetShaderColor(
		0.04f,
		0.04f,
		0.045f,
		1.0f);

	SetShaderMaterial("metal");

	m_basicMeshes->DrawBoxMesh();


	/****************************************************************/
	// MONITOR STAND - MAIN POST
	/****************************************************************/
	scaleXYZ =
		glm::vec3(
			0.34f,
			1.42f,
			0.34f);

	positionXYZ =
		glm::vec3(
			1.35f,
			0.92f,
			-2.10f);

	SetTransformations(
		scaleXYZ,
		0.0f,
		0.0f,
		0.0f,
		positionXYZ);

	SetShaderColor(
		0.04f,
		0.04f,
		0.045f,
		1.0f);

	SetShaderMaterial("metal");

	m_basicMeshes->DrawBoxMesh();


	/****************************************************************/
	// MONITOR STAND - BASE HUB
	/****************************************************************/
	scaleXYZ =
		glm::vec3(
			0.90f,
			0.13f,
			0.68f);

	positionXYZ =
		glm::vec3(
			1.35f,
			0.16f,
			-1.55f);

	SetTransformations(
		scaleXYZ,
		0.0f,
		0.0f,
		0.0f,
		positionXYZ);

	SetShaderColor(
		0.04f,
		0.04f,
		0.045f,
		1.0f);

	SetShaderMaterial("metal");

	m_basicMeshes->DrawBoxMesh();


	/****************************************************************/
	// MONITOR STAND - LEFT FOOT
	/****************************************************************/
	scaleXYZ =
		glm::vec3(
			0.23f,
			0.11f,
			1.38f);

	positionXYZ =
		glm::vec3(
			0.72f,
			0.14f,
			-0.73f);

	SetTransformations(
		scaleXYZ,
		0.0f,
		-27.0f,
		0.0f,
		positionXYZ);

	SetShaderColor(
		0.04f,
		0.04f,
		0.045f,
		1.0f);

	SetShaderMaterial("metal");

	m_basicMeshes->DrawBoxMesh();


	/****************************************************************/
	// MONITOR STAND - RIGHT FOOT
	/****************************************************************/
	positionXYZ =
		glm::vec3(
			1.98f,
			0.14f,
			-0.73f);

	SetTransformations(
		scaleXYZ,
		0.0f,
		27.0f,
		0.0f,
		positionXYZ);

	SetShaderColor(
		0.04f,
		0.04f,
		0.045f,
		1.0f);

	SetShaderMaterial("metal");

	m_basicMeshes->DrawBoxMesh();


	/****************************************************************/
	// EXTERNAL KEYBOARD
	/****************************************************************/
	scaleXYZ =
		glm::vec3(
			4.75f,
			0.10f,
			1.20f);

	positionXYZ =
		glm::vec3(
			0.10f,
			0.18f,
			2.75f);

	SetTransformations(
		scaleXYZ,
		0.0f,
		-3.0f,
		0.0f,
		positionXYZ);

	SetShaderMaterial("plastic");
	SetTextureUVScale(1.0f, 1.0f);

	// Dark plastic sides and bottom.
	SetShaderTexture("plastic");

	m_basicMeshes->DrawBoxMeshSide(
		ShapeMeshes::box_back);

	m_basicMeshes->DrawBoxMeshSide(
		ShapeMeshes::box_bottom);

	m_basicMeshes->DrawBoxMeshSide(
		ShapeMeshes::box_left);

	m_basicMeshes->DrawBoxMeshSide(
		ShapeMeshes::box_right);

	m_basicMeshes->DrawBoxMeshSide(
		ShapeMeshes::box_front);

	// Keyboard image only on the upper face.
	SetShaderTexture(
		"externalKeyboard");

	m_basicMeshes->DrawBoxMeshSide(
		ShapeMeshes::box_top);


	/****************************************************************/
	// MOUSE PAD
	/****************************************************************/
	scaleXYZ =
		glm::vec3(
			1.78f,
			0.04f,
			1.42f);

	positionXYZ =
		glm::vec3(
			4.10f,
			0.07f,
			2.75f);

	SetTransformations(
		scaleXYZ,
		0.0f,
		0.0f,
		0.0f,
		positionXYZ);

	SetShaderTexture("mousepad");
	SetTextureUVScale(2.0f, 2.0f);
	SetShaderMaterial("plastic");

	m_basicMeshes->DrawBoxMesh();


	/****************************************************************/
	// MOUSE BODY
	/****************************************************************/
	scaleXYZ =
		glm::vec3(
			0.43f,
			0.14f,
			0.65f);

	positionXYZ =
		glm::vec3(
			4.10f,
			0.23f,
			2.75f);

	SetTransformations(
		scaleXYZ,
		0.0f,
		-8.0f,
		0.0f,
		positionXYZ);

	SetShaderColor(
		0.06f,
		0.06f,
		0.07f,
		1.0f);

	SetShaderMaterial("plastic");

	m_basicMeshes->DrawSphereMesh();


	/****************************************************************/
	// MOUSE SCROLL WHEEL
	/****************************************************************/
	scaleXYZ =
		glm::vec3(
			0.055f,
			0.08f,
			0.055f);

	positionXYZ =
		glm::vec3(
			4.10f,
			0.37f,
			2.52f);

	SetTransformations(
		scaleXYZ,
		0.0f,
		0.0f,
		90.0f,
		positionXYZ);

	SetShaderColor(
		0.40f,
		0.40f,
		0.42f,
		1.0f);

	SetShaderMaterial("metal");

	m_basicMeshes->DrawCylinderMesh();
}