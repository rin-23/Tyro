//
//  RAES2Renderer.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-01-08.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include "Wm5Vector4.h"
#include "RAEnginePrerequisites.h"



namespace tyro
{  
//	class ES2Context;
    
	class ES2Renderer
    {
    public:
		ES2Renderer();
		ES2Renderer(ES2Context* glContext);

        ~ES2Renderer();
        
        void RenderVisibleSet(const VisibleSet* visibleSet, const Camera* camera);
        
        TextureBuffer* RenderVisibleSetToTexture(const VisibleSet* visualSet, const Camera* camera);

		void SetClearColor(const Wm5::Vector4f& clearColor);

		void ClearScreen();
    
	private:
 
        ES2Context* mContext;

		Wm5::Vector4f mClearColor;
		int mViewWidth;
		int mViewHeight;
	
		void RenderPrimitive(const ES2Renderable* renderable) const;
		
		void SetShader(const ES2ShaderProgram* shaderProgram) const;

		void SetTexture(const ES2Texture2DSPtr texture) const;

		void SetAlphaState(const ES2VisualEffectSPtr ve) const;

		void SetCullState(const ES2VisualEffectSPtr ve) const;

		void SetDepthState(const ES2VisualEffectSPtr ve) const;

		void SetPolygonOffset(const ES2VisualEffectSPtr ve) const;

		void UpdateUniforms(const ES2ShaderUniforms* uniforms) const;

		void PrepareVertexBuffers(const ES2VertexFormat* vertexFormat, const ES2VertexHardwareBufferSPtr vertexBufffer) const;

		void DrawPrimitive(const ES2Renderable* renderable) const;

		void DisableAttributes(const ES2VertexFormat* vertexFormat, const ES2VertexHardwareBufferSPtr vertexBufffer) const;
	
	};
}
