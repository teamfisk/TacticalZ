
#include "Rendering/FrameBuffer.h"


BufferResource::BufferResource(GLuint* resourceHandle, GLenum resourceType, GLenum attachment)
{
    m_ResourceHandle = resourceHandle;
    m_ResourceType = resourceType;
    m_Attachment = attachment;
}

Texture2D::~Texture2D()
{
    if (m_ResourceHandle != 0) {
        glDeleteTextures(1, m_ResourceHandle);
    }
}


RenderBuffer::~RenderBuffer()
{
    if (m_ResourceHandle != 0) {
        glDeleteRenderbuffers(1, m_ResourceHandle);
    }
}


FrameBuffer::~FrameBuffer()
{
    if (m_BufferHandle != 0) {
        glDeleteFramebuffers(1, &m_BufferHandle);
    }
}

void FrameBuffer::AddResource(std::shared_ptr<BufferResource> resource)
{
    m_Resources.push_back(resource);
}

void FrameBuffer::Generate()
{
    std::vector<GLenum> attachments;

    glGenFramebuffers(1, &m_BufferHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, m_BufferHandle);

    for (auto it = m_Resources.begin(); it != m_Resources.end(); it++) {
        switch ((*it)->m_ResourceType) {
        case GL_TEXTURE_2D:
            glFramebufferTexture2D(GL_FRAMEBUFFER, (*it)->m_Attachment, (*it)->m_ResourceType, *(*it)->m_ResourceHandle, 0);
            GLERROR("FrameBuffer generate: glFramebufferTexture2D");

            break;
        case GL_RENDERBUFFER:
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, (*it)->m_Attachment, (*it)->m_ResourceType, *(*it)->m_ResourceHandle);
            GLERROR("FrameBuffer generate: glFramebufferRenderbuffer");
            if ( (*it)->m_Attachment != GL_COLOR_ATTACHMENT0 ||
                (*it)->m_Attachment != GL_DEPTH_ATTACHMENT ||
                (*it)->m_Attachment != GL_STENCIL_ATTACHMENT)
            {
                LOG_ERROR("RenderBuffer Attachment not valid.");
            }
            break;
        }
        

        if ((*it)->m_Attachment != GL_DEPTH_ATTACHMENT) {
            attachments.push_back((*it)->m_Attachment);
        }
    }
        
   
    
    GLenum* bufferTextures = &attachments[0];
    glDrawBuffers(1, bufferTextures);

    if (GLenum frameBufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOG_ERROR("FrameBuffer incomplete: 0x%x\n", frameBufferStatus);
        exit(EXIT_FAILURE);
    }
}

void FrameBuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_BufferHandle);
}

void FrameBuffer::Unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint FrameBuffer::GetHandle()
{
    return m_BufferHandle;
}
