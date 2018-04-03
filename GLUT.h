//
//  GLUT.h
//
//  Created by Andrew Barker on 9/20/16.
//
//

#ifndef GLUT_h
#define GLUT_h

//#ifdef __APPLE__
//  #include <GLUT/glut.h>
//#else // windows or linux
//  #define FREEGLUT_STATIC 1
//  #include <GL/glut.h>
//#endif

//#include <cassert>
#include "Points.h"

// screw linking to other libraries when doing cross-platform dev...
namespace glpp {

static void lookAt (const PointXYZ<float>& eye,
                    const PointXYZ<float>& lookAt,
                    const PointXYZ<float>& up)
{
    cauto forward = normalized((lookAt - eye));
    cauto right = normalized(crossProduct(forward, up));
    cauto nUp = crossProduct(right, forward);

    GLfloat m[] = {
        right.x, nUp.x, -forward.x, 0,
        right.y, nUp.y, -forward.y, 0,
        right.z, nUp.z, -forward.z, 0,
        0,           0,          0, 1
    };

    glMultMatrixf(m);
    glTranslatef(-eye.x, -eye.y, -eye.z);
}

// Replaces gluPerspective. Sets the frustum to perspective mode.
// fovY     - Field of vision in degrees in the y direction
// aspect   - Aspect ratio of the viewport (w/h)
// zNear    - The near clipping distance
// zFar     - The far clipping distance
static void perspective (const GLdouble fovY, const GLdouble aspect,
                         const GLdouble zNear, const GLdouble zFar)
{
    //const GLdouble pi = 3.1415926535897932384626433832795;
    cauto fH = std::tan(0.5 * toRadians(fovY)) * zNear;
    cauto fW = fH * aspect;
    glFrustum(-fW, fW, -fH, fH, zNear, zFar);
}

static void pickMatrix(const GLdouble x, const GLdouble y,
                       const GLdouble width, const GLdouble height,
                       const GLint viewport[4])
{
    GLfloat m[16];
    GLfloat sx, sy;
    GLfloat tx, ty;

    sx = viewport[2] / width;
    sy = viewport[3] / height;
    tx = (viewport[2] + 2.0 * (viewport[0] - x)) / width;
    ty = (viewport[3] + 2.0 * (viewport[1] - y)) / height;

  #define M(row, col) m[col*4+row]
      M(0, 0) = sx;
      M(0, 1) = 0.0;
      M(0, 2) = 0.0;
      M(0, 3) = tx;
      M(1, 0) = 0.0;
      M(1, 1) = sy;
      M(1, 2) = 0.0;
      M(1, 3) = ty;
      M(2, 0) = 0.0;
      M(2, 1) = 0.0;
      M(2, 2) = 1.0;
      M(2, 3) = 0.0;
      M(3, 0) = 0.0;
      M(3, 1) = 0.0;
      M(3, 2) = 0.0;
      M(3, 3) = 1.0;
  #undef M

    glMultMatrixf(m);
}

// thank you datenwolf!  http://stackoverflow.com/questions/5988686/creating-a-3d-sphere-in-opengl-using-visual-c
class SolidSphere
{
private:
    std::vector<GLfloat> vertices;
    std::vector<GLfloat> normals;
    std::vector<GLfloat> texcoords;
    std::vector<GLushort> indices;

public:
    SolidSphere(float radius, unsigned int sectors, unsigned int rings)
    {
        cauto R = 1.0f / (float)(rings-1);
        cauto S = 1.0f / (float)(sectors-1);
        int r, s;
        
        vertices.resize(rings * sectors * 3);
        normals.resize(rings * sectors * 3);
        texcoords.resize(rings * sectors * 2);
        auto v = vertices.begin();
        auto n = normals.begin();
        auto t = texcoords.begin();
        for (r = 0; r < rings; r++) {
            for (s = 0; s < sectors; s++) {
                cfloat y = std::sin(-M_PI_2 + M_PI * r * R);
                cfloat x = std::cos(2*M_PI * s * S) * std::sin(M_PI * r * R);
                cfloat z = std::sin(2*M_PI * s * S) * std::sin(M_PI * r * R);

                *t++ = s * S;
                *t++ = r * R;

                *v++ = x * radius;
                *v++ = y * radius;
                *v++ = z * radius;

                *n++ = x;
                *n++ = y;
                *n++ = z;
            }
        }
        
        indices.resize(rings * sectors * 4);
        auto i = indices.begin();
        for (r = 0; r < rings-1; r++) {
            for (s = 0; s < sectors-1; s++) {
                *i++ = r * sectors + s;
                *i++ = r * sectors + (s+1);
                *i++ = (r+1) * sectors + (s+1);
                *i++ = (r+1) * sectors + s;
            }
        }
    }

    void draw(GLfloat x, GLfloat y, GLfloat z) const
    {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glTranslatef(x, y, z);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glVertexPointer(3, GL_FLOAT, 0, &vertices[0]);
        glNormalPointer(GL_FLOAT, 0, &normals[0]);
        glTexCoordPointer(2, GL_FLOAT, 0, &texcoords[0]);
        glDrawElements(GL_QUADS, indices.size(), GL_UNSIGNED_SHORT, &indices[0]);
        
        glPopMatrix();
    }
};

//template <class T>
//void perspective(const T fovy, const T aspect, const T zNear, const T zFar)
//{
//    assert(abs(aspect - std::numeric_limits<T>::epsilon()) > static_cast<T>(0));
//
//    const T tanHalfFovy = std::tan(fovy / static_cast<T>(2));
//
//    //tmat4x4<T, defaultp> Result(static_cast<T>(0));
//    GLfloat m[4][4] = {0};
//    m[0][0] = static_cast<T>(1) / (aspect * tanHalfFovy);
//    m[1][1] = static_cast<T>(1) / (tanHalfFovy);
//    m[2][3] = - static_cast<T>(1);
//
////#		if GLM_DEPTH_CLIP_SPACE == GLM_DEPTH_ZERO_TO_ONE
////        Result[2][2] = zFar / (zNear - zFar);
////        Result[3][2] = -(zFar * zNear) / (zFar - zNear);
////#		else
//        m[2][2] = - (zFar + zNear) / (zFar - zNear);
//        m[3][2] = - (static_cast<T>(2) * zFar * zNear) / (zFar - zNear);
////#		endif
////    return Result;
//    glMultMatrixf(&m[0][0]);
//}

}
#endif /* GLUT_h */
