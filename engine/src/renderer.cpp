//
// Created by koerriva on 2021/12/21.
//

#include <iostream>
#include "log.h"
#include "renderer.h"
#include "font.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#define PI 3.1415926

namespace wx {
    Renderer::Renderer(/* args */)
    = default;

    Renderer::~Renderer()
    {
        std::cout << "Drop Renderer" << std::endl;
    }

    void Renderer::Init(){
        glClearColor(0.f,0.f,0.f,1.0f);
        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
    }

    void Renderer::SetWireframeMode(){
        WIREFRAME_MODE = true;
        SHADER_MODE = false;
    }

    void Renderer::SetShaderMode() {
        WIREFRAME_MODE = false;
        SHADER_MODE = true;
    }

    void Renderer::Render(const Window* window,const Camera* camera,const vector<Mesh>& meshList,const vector<Texture>& textures,ShaderProgram* shaderProgram){
        glEnable(GL_DEPTH_TEST);
//        glEnable(GL_CULL_FACE);
//        glCullFace(GL_BACK);
        if(WIREFRAME_MODE){
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        if(SHADER_MODE){
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        glActiveTexture(GL_TEXTURE0);
        textures[0].Bind();

        shaderProgram->Bind();
        auto time = (float)Window::GetTimeInSecond();
        shaderProgram->SetFloat("time",time);

        float aspect = window->GetAspect();
        glm::mat4 P,V(1.0f),M(1.0f);
        P = glm::perspective(glm::radians(60.f),aspect,.1f,99999.f);
        V = camera->GetViewMatrix();
        M = glm::rotate(M,time*0.1f,glm::vec3(0,1,0));
        M = glm::scale(M,glm::vec3(1.0f));

        shaderProgram->SetMat4("P", reinterpret_cast<float *>(&P));
        shaderProgram->SetMat4("V", reinterpret_cast<float *>(&V));
        shaderProgram->SetMat4("M", reinterpret_cast<float *>(&M));

        vec4 base_color{1.0f};
        shaderProgram->SetVec4("base_color", value_ptr(base_color));

        for (size_t i = 0; i < meshList.size(); i++)
        {
            auto& mesh = meshList[i];
            mesh.Draw();
        }

        ShaderProgram::Unbind();

        glDisable(GL_DEPTH_TEST);
    }

    void Renderer::Render(const Window* window,const Camera* camera,Terrain* terrain,ShaderProgram* shaderProgram){
        glEnable(GL_DEPTH_TEST);
//        glEnable(GL_CULL_FACE);
//        glCullFace(GL_BACK);
        if(WIREFRAME_MODE){
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        if(SHADER_MODE){
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        float aspect = window->GetAspect();
        glm::mat4 P,V(1.0f),M(1.0f);
        P = glm::perspective(glm::radians(60.f),aspect,.1f,1000.f);
        V = camera->GetViewMatrix();
        M = glm::scale(M,glm::vec3(1.0f));

        shaderProgram->Bind();
        shaderProgram->SetMat4("P", reinterpret_cast<float *>(&P));
        shaderProgram->SetMat4("V", reinterpret_cast<float *>(&V));
        shaderProgram->SetMat4("M", reinterpret_cast<float *>(&M));

        glm::vec4 base_color{1.0f};
        shaderProgram->SetVec4("base_color", value_ptr(base_color));

        terrain->Draw();

        ShaderProgram::Unbind();

        glDisable(GL_DEPTH_TEST);
    }

    //Mesh
    Mesh::Mesh(vector<float> &vertices,vector<unsigned>& indices,vector<float> &normals,vector<float> &texCoords,vector<float>& colors) {
        this->vertices = vertices;
        this->indices = indices;
        this->normals = normals;
        this->texCoords = texCoords;

        glGenVertexArrays(1,&vao);
        glBindVertexArray(vao);

        cout << "vertices :" << vertices.size() << endl;
        unsigned int vbo;

        //vertices
        glGenBuffers(1,&vbo);
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(float),vertices.data(),GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),nullptr);
        vbos.push_back(vbo);

        //normals
        glGenBuffers(1,&vbo);
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER,normals.size()*sizeof(float),normals.data(),GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,3*sizeof(float),nullptr);
        vbos.push_back(vbo);

        //texcoords
        glGenBuffers(1,&vbo);
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER,texCoords.size()*sizeof(float),texCoords.data(),GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,2*sizeof(float),nullptr);
        vbos.push_back(vbo);

        if(!colors.empty()){
            //colors
            glGenBuffers(1,&vbo);
            glBindBuffer(GL_ARRAY_BUFFER,vbo);
            glBufferData(GL_ARRAY_BUFFER,colors.size()*sizeof(float),colors.data(),GL_STATIC_DRAW);
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3,3,GL_FLOAT,GL_FALSE,3*sizeof(float),nullptr);
            vbos.push_back(vbo);
        }

        glGenBuffers(1,&ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,indices.size()*sizeof(unsigned),indices.data(),GL_STATIC_DRAW);

        glBindVertexArray(0);
    }

    Mesh::~Mesh(){
        cout << "Drop Mesh" << endl;
    }

    void Mesh::Draw() const {
        glBindVertexArray(vao);
//        glDrawArrays(GL_TRIANGLES,0,3);
        glDrawElements(GL_TRIANGLES,indices.size(),GL_UNSIGNED_INT,nullptr);
        glBindVertexArray(0);
    }

    void Mesh::Cleanup() const {
        cout << "Clean Mesh " << vao << endl;
        glDeleteVertexArrays(1,&vao);
        glDeleteBuffers(vbos.size(),vbos.data());
    }

    Mesh Mesh::Sphere(float r,int sectors,int stacks) {
        vector<float> vertices;
        vector<float> colors;
        vector<unsigned int> indices;
        vector<float> normals;
        vector<float> texCoords;

        float sectorStep = float(2.*PI) / float(sectors);//圆周等分
        auto stackStep = float(PI / stacks); //半圆等分
        float len = 1.0f/float(r);

        ofstream file("log.txt");
        for (int i = 0; i <= stacks; ++i) {
            auto stackAngle = float(PI/2 - float(i)*stackStep); //垂直角
            float y = r*sinf(stackAngle);
            float xz = r*cosf(stackAngle);

            for (int j = 0; j <= sectors; ++j) {
                float sectorAngle = float(j)*sectorStep; //水平角
                float x = xz*cosf(sectorAngle);
                float z = xz*sinf(sectorAngle);

                //顶点坐标
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);

                vec3 color(1.0,1.0,1.0);
                colors.push_back(color.r);
                colors.push_back(color.g);
                colors.push_back(color.b);

                //贴图坐标
                float s = float(j)/float(sectors);
                float t = float(i)/float(stacks);
                texCoords.push_back(s);
                texCoords.push_back(t);

                //法线
                normals.push_back(x*len);
                normals.push_back(y*len);
                normals.push_back(z*len);
            }
        }

        for (int v = 0; v < stacks; ++v) {
            int v0 = v*(sectors+1);//起点
            int v1 = v0+sectors+1;//终点
            for (int h = 0; h < sectors; ++h,++v0,++v1) {
                if(v!=0){
                    indices.push_back(v0);
                    indices.push_back(v1);
                    indices.push_back(v0+1);
                }
                if(v!=(stacks-1)){
                    indices.push_back(v0+1);
                    indices.push_back(v1);
                    indices.push_back(v1+1);
                }
            }
        }

        return Mesh(vertices,indices,normals,texCoords,colors);
    }

    void Mesh::DumpPNGFile(int width, int height, vector<float> &colors) {
        int pos=0,comps=3;
        vector<unsigned char> buffer;
        buffer.resize(width*height*comps);

        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                buffer[pos] = static_cast<int>(colors[pos]*255.99);
                pos++;
                buffer[pos] = static_cast<int>(colors[pos]*255.99);
                pos++;
                buffer[pos] = static_cast<int>(colors[pos]*255.99);
                pos++;
            }
        }

        stbi_write_png("terrain.png",width,height,comps,buffer.data(),0);
    }

    /**
     * texture
     * @param buffer
     * @param len
     */
    Texture::Texture(const unsigned char *buffer, int len) {
        this->buffer = buffer;
        this->len = len;
        unsigned char* data = stbi_load_from_memory(buffer,len,&width,&height,&comp,0);
        WX_CORE_INFO("Image Info width={}, height={}, channels={}",width,height,comp);
        glGenTextures(1,&texture);
        //为当前绑定的纹理对象设置环绕、过滤方式
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //生成纹理
        glBindTexture(GL_TEXTURE_2D,texture);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }

    void Texture::Bind() const {
        glBindTexture(GL_TEXTURE_2D,texture);
    }

    void Texture::Cleanup() {
        cout << "Cleanup Texture" << endl;
        glDeleteTextures(1,&texture);
    }

    /**
     * ShaderProgram
     * @param type
     * @return
     */

    ShaderProgram::ShaderProgram(const char *name) {
        this->vertexSource = AssetsLoader::LoadShader(name, VERTEX_SHADER);
        this->fragmentSource = AssetsLoader::LoadShader(name, FRAGMENT_SHADER);
        Upload();
    }

    unsigned int ShaderProgram::CreateShader(GLuint type) {
        unsigned int shader = glCreateShader(type);
        if (type==GL_VERTEX_SHADER){
            glShaderSource(shader,1,&vertexSource, nullptr);

        }else if(type==GL_FRAGMENT_SHADER){
            glShaderSource(shader,1,&fragmentSource, nullptr);
        }else{
            WX_CORE_CRITICAL("Unsupported Shader type {}",type);
            exit(-1001);
        }
        glCompileShader(shader);
        int success;
        char info[512];
        glGetShaderiv(shader,GL_COMPILE_STATUS,&success);
        if(!success){
            glGetShaderInfoLog(shader,512, nullptr,info);
            WX_CORE_CRITICAL(info);
            exit(-1002);
        }
        return shader;
    }

    unsigned int ShaderProgram::CreateProgram(unsigned int vertShader, unsigned int fragShader) {
        unsigned int program = glCreateProgram();
        glAttachShader(program,vertShader);
        glAttachShader(program,fragShader);
        glLinkProgram(program);
        int success;
        char info[512];
        glGetProgramiv(program,GL_LINK_STATUS, &success);
        if(!success){
            glGetProgramInfoLog(program,512, nullptr,info);
            WX_CORE_CRITICAL(info);
            exit(-1002);
        }
        return program;
    }

    void ShaderProgram::Upload() {
        unsigned vertexShader = CreateShader(GL_VERTEX_SHADER);
        unsigned fragmentShader = CreateShader(GL_FRAGMENT_SHADER);
        shaderProgram = CreateProgram(vertexShader,fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    void ShaderProgram::Bind() const {
        glUseProgram(shaderProgram);
    }

    void ShaderProgram::Unbind() {
        glUseProgram(0);
    }

    void ShaderProgram::Cleanup() const {
        cout << "Clean Program " << shaderProgram << endl;
        glDeleteProgram(shaderProgram);
    }

    void ShaderProgram::SetFloat(string name, float value) {
        int location = 0;
        if(uniforms.count(name)==0){
            cout << "Find Uniform : " << name << endl;
            location = glGetUniformLocation(shaderProgram,name.c_str());
            cout << "Uniform[" << name << "] Location=" << location << endl;
            uniforms[name]=location;
        }else{
            location = uniforms[name];
        }
        glUniform1f(location,value);
    }

    void ShaderProgram::SetMat4(string name, float *value) {
        int location = 0;
        if(uniforms.count(name)==0){
            cout << "Find Uniform : " << name << endl;
            location = glGetUniformLocation(shaderProgram,name.c_str());
            cout << "Uniform[" << name << "] Location=" << location << endl;
            uniforms[name]=location;
        }else{
            location = uniforms[name];
        }
        glUniformMatrix4fv(location,1,GL_FALSE,value);
    }
    void ShaderProgram::SetVec4(string name, float *value) {
        int location = 0;
        if(uniforms.count(name)==0){
            cout << "Find Uniform : " << name << endl;
            location = glGetUniformLocation(shaderProgram,name.c_str());
            cout << "Uniform[" << name << "] Location=" << location << endl;
            uniforms[name]=location;
        }else{
            location = uniforms[name];
        }
        glUniform4fv(location,1,value);
    }


    void ShaderProgram::SetVec3(string name, float *value) {
        int location = 0;
        if(uniforms.count(name)==0){
            cout << "Find Uniform : " << name << endl;
            location = glGetUniformLocation(shaderProgram,name.c_str());
            cout << "Uniform[" << name << "] Location=" << location << endl;
            uniforms[name]=location;
        }else{
            location = uniforms[name];
        }
        glUniform3fv(location,1,value);
    }

    ShaderProgram::~ShaderProgram() {
        cout << "Drop ShaderProgram" << endl;
    }

    QuadTreeNode *Terrain::CreateNewChunk(size_t depth, vec3 center, float bound, QuadTreeNode *parent) {
        //          Logger::Info("Create QuadTreeNode ({},{},{}),{:.4f}",center.x,center.y,center.z,bound);
        QuadTreeNode node;
        node.depth = depth;
        node.center = center;
        node.bound = bound;
        node.parent = parent;
//          Logger::Info("Set Node[{}] Bound {:.4f}",node.id,bound);
        vec3 top_left = center+vec3(-bound/2,0.f,-bound/2);
        vec3 top_right = center+vec3(bound/2,0.f,-bound/2);
        vec3 bot_left = center+vec3(-bound/2,0.f,bound/2);
        vec3 bot_right = center+vec3(bound/2,0.f,bound/2);
        vector<vec3> buffer;
        buffer.push_back(top_left);
        buffer.push_back(top_right);
        buffer.push_back(bot_right);
        buffer.push_back(bot_left);
        auto *ptr = reinterpret_cast<float *>(buffer.data());
        for (int i = 0; i < 12; ++i) {
            node.patch.points[i] = *(ptr + i);
        }
        node.patch.Upload();

        node.id = next_id++;
        chunks[node.id]=node;
        return chunks+node.id;
    }

    void Terrain::SplitChunk(QuadTreeNode *parent) {
        float childBound = parent->bound/2;
        vec3 NW_OFFSET(-childBound/2,0.f,-childBound/2);
        vec3 NE_OFFSET(childBound/2,0.f,-childBound/2);
        vec3 SW_OFFSET(-childBound/2,0.f,childBound/2);
        vec3 SE_OFFSET(childBound/2,0.f,childBound/2);
        parent->children[0] = CreateNewChunk(parent->depth+1,parent->center+NW_OFFSET,childBound,parent);
        parent->children[1] = CreateNewChunk(parent->depth+1,parent->center+NE_OFFSET,childBound,parent);
        parent->children[2] = CreateNewChunk(parent->depth+1,parent->center+SW_OFFSET,childBound,parent);
        parent->children[3] = CreateNewChunk(parent->depth+1,parent->center+SE_OFFSET,childBound,parent);
        parent->children_count=4;
    }

    void Terrain::SplitChunk(QuadTreeNode *parent, int depth) {
        if(parent->depth<depth){
            if(parent->children_count==0){
                SplitChunk(parent);
            }else{
                for (int i = 0; i < parent->children_count; ++i) {
                    SplitChunk(parent->children[i],depth);
                }
            }
        }
    }

    tuple<float,vec3> TerrainFaceColor[]={
            make_tuple(1.0000f,vec3(1.0f,1.0f,1.0f)),//snow
            make_tuple(0.7500f,vec3(128.f/255.99f,128.f/255.99f,128.f/255.99f)),//rock
            make_tuple(0.3750f,vec3(224.f/255.99f,224.f/255.99f,0.f)),//dirt
            make_tuple(0.1250f,vec3(32.f/255.99f,160.f/255.99f,0.f)),//grass
            make_tuple(0.0625f,vec3(240.f/255.99f,240.f/255.99f,64.f)),//sand
            make_tuple(0.0000f,vec3(0.f,128.f/255.99f,1.f)),//shore
            make_tuple(-0.2500f,vec3(0.f,0.f,1.f)),//shallow water
            make_tuple(-1.000f,vec3(0.f,0.f,128.f/255.f)),//deep water
    };

    Terrain::Terrain() {
        cout << "QuadTreeNode Size:" << sizeof(QuadTreeNode) << endl;
        vec3 origin(0.f,0.f,0.f);
        float bound = 10.f;
        vec3 up_offset(0,0,-bound),down_offset(0,0,bound),left_offset(-bound,0,0),right_offset(bound,0,0);

        int init_depth=1;
        auto root = CreateNewChunk(init_depth,origin,bound, nullptr);
        root->neighbors[0] = CreateNewChunk(init_depth,origin+up_offset,bound,nullptr);
        root->neighbors[1] = CreateNewChunk(init_depth,origin+down_offset,bound,nullptr);
        root->neighbors[2] = CreateNewChunk(init_depth,origin+left_offset,bound,nullptr);
        root->neighbors[3] = CreateNewChunk(init_depth,origin+right_offset,bound,nullptr);
        root->neighbors_count = 4;

        target_chunk = root;
        target_depth = init_depth;

        WX_CORE_INFO("Root Node[{}] Depth[{}] {:.4f}",root->id,root->depth,root->bound);
    }

    void Patch::Upload() {
        unsigned int vao,vbo,ebo;
        glGenVertexArrays(1,&vao);
        glBindVertexArray(vao);

        glGenBuffers(1,&vbo);
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER,sizeof(this->points),this->points,GL_STATIC_DRAW);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float), nullptr);
        glEnableVertexAttribArray(0);

        glGenBuffers(1,&ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(this->indices),this->indices,GL_STATIC_DRAW);

        glBindVertexArray(0);

        this->vao = vao;
        this->vbo = vbo;
        this->ebo = ebo;
        this->loaded = true;
    }

    void Patch::Draw() {
        if(this->loaded){
            glBindVertexArray(this->vao);
            glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
        }
    }

    /**
     * Debug
     */
    struct Character{
        bool cached=false;
        GLuint texture;
        ivec2 size;
        ivec2 bearing;
        GLuint advance;
    };

    Debug::Debug() {
        shaderProgram = new ShaderProgram("font");
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(GLfloat), nullptr, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,4*sizeof(float),nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void Debug::PrintScreen(vec2 pos, const char* text, vec3 color) {
        glEnable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        shaderProgram->Bind();
        shaderProgram->SetVec3("color", reinterpret_cast<float *>(&color));
        mat4 P = ortho(0.f,1280.f,720.f,0.f);
        shaderProgram->SetMat4("P",reinterpret_cast<float *>(&P));
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(VAO);

        std::string chars = std::string(text);
        for (auto it=chars.begin();it!=chars.end();++it) {
            Character c = Font::GetChar(*it);
            float x = pos.x+c.bearing.x*1.0f;
            float y = pos.y+Font::PIXEL_SIZE+float((c.size.y-c.bearing.y))*1.0f;
            auto w = float(c.size.x);
            auto h = float(c.size.y);

            //6个顶点
            vertices ={
                    x,y-h,0.0,0.0,
                    x,y,0.0,1.0,
                    x+w,y,1.0,1.0,

                    x,y-h,0.0,0.0,
                    x+w,y,1.0,1.0,
                    x+w,y-h,1.0,0.0,
            };

            pos.x += c.advance>>6;

            glBindTexture(GL_TEXTURE_2D,c.texture);
            glBindBuffer(GL_ARRAY_BUFFER,VBO);
            glBufferSubData(GL_ARRAY_BUFFER,0,vertices.size()*sizeof(float),vertices.data());
            glDrawArrays(GL_TRIANGLES,0,6);
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D,0);

        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);
    }
}
