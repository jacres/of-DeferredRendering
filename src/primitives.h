#ifndef PRIMITIVES_H_INCLUDED
#define PRIMITIVES_H_INCLUDED

class Primitives {

public:
  static void addFace(ofMesh& mesh, ofVec3f a, ofVec3f b, ofVec3f c) {
    mesh.addVertex(a);
    mesh.addVertex(b);
    mesh.addVertex(c);

    ofVec3f p0 = (a-b).getNormalized();
    ofVec3f p1 = (b-c).getNormalized();

    ofVec3f n = p0.getCrossed(p1).getNormalized();
    mesh.addNormal(n);
    mesh.addNormal(n);
    mesh.addNormal(n);

    //    ofFloatColor col = ofFloatColor(ofRandomuf(), ofRandomuf(), ofRandomuf(), 0.2f);
    ofFloatColor col = ofFloatColor(1.0f, 1.0f, 1.0f, 1.0f);
    mesh.addColor(col);
    mesh.addColor(col);
    mesh.addColor(col);
  }

  //--------------------------------------------------------------
  static void addFace(ofMesh& mesh, ofVec3f a, ofVec3f b, ofVec3f c, ofVec3f d) {
    addFace(mesh, a, b, c);
    addFace(mesh, a, c, d);
  }

  static ofMesh getBoxMesh(float w, float h, float depth) {

    float hW = w/2.0f;
    float hH = h/2.0f;
    float hD = depth/2.0f;

    ofMesh boxMesh;

    // BACK
    ofVec3f p0 = ofVec3f(-hW, +hH, hD); // p0
    ofVec3f p1 = ofVec3f(+hW, +hH, hD); // p1
    ofVec3f p2 = ofVec3f(+hW, -hH, hD); // p2
    ofVec3f p3 = ofVec3f(-hW, -hH, hD); // p3

    // FRONT
    ofVec3f p4 = ofVec3f(-hW, +hH, -hD); // p4
    ofVec3f p5 = ofVec3f(+hW, +hH, -hD); // p5
    ofVec3f p6 = ofVec3f(+hW, -hH, -hD); // p6
    ofVec3f p7 = ofVec3f(-hW, -hH, -hD); // p7


    // CW WINDING
    addFace(boxMesh, p3, p2, p1, p0); // BOTTOM
    addFace(boxMesh, p7, p3, p0, p4); // LEFT
    addFace(boxMesh, p0, p1, p5, p4); // BACK
    addFace(boxMesh, p4, p5, p6, p7); // FRONT
    addFace(boxMesh, p7, p6, p2, p3); // HORIZONTAL DEPTH SLICES (TOP)
    addFace(boxMesh, p2, p6, p5, p1); // VERTICAL DEPTH SLICES (RIGHT)

    return boxMesh;
  }

  static ofMesh getSphereMesh(int res) {
    ofMesh sphereMesh;

    int n = res * 2;
    float ndiv2=(float)n/2;

    /*
     Original code by Paul Bourke
     A more efficient contribution by Federico Dosil (below)
     Draw a point for zero radius spheres
     Use CCW facet ordering
     http://paulbourke.net/texture_colour/texturemap/
     */

    float theta2 = TWO_PI;
    float phi1 = -HALF_PI;
    float phi2 = HALF_PI;
    float r = 1.f; // normalize the verts //

    sphereMesh.clear();
    sphereMesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);

    int i, j;
    float theta1 = 0.f;
    float jdivn,j1divn,idivn,dosdivn,unodivn=1/(float)n,t1,t2,t3,cost1,cost2,cte1,cte3;
    cte3 = (theta2-theta1)/n;
    cte1 = (phi2-phi1)/ndiv2;
    dosdivn = 2*unodivn;
    ofVec3f e,p,e2,p2;

    if (n < 0){
      n = -n;
      ndiv2 = -ndiv2;
    }
    if (n < 4) {n = 4; ndiv2=(float)n/2;}
    if(r <= 0) r = 1;

      t2=phi1;
      cost2=cos(phi1);
      j1divn=0;

      ofVec3f vert, normal;
    ofVec2f tcoord;

    for (j=0;j<ndiv2;j++) {
      t1 = t2;
      t2 += cte1;
      t3 = theta1 - cte3;
      cost1 = cost2;
      cost2 = cos(t2);
      e.y = sin(t1);
      e2.y = sin(t2);
      p.y = r * e.y;
      p2.y = r * e2.y;

      idivn=0;
      jdivn=j1divn;
      j1divn+=dosdivn;
      for (i=0;i<=n;i++) {
        t3 += cte3;
        e.x = cost1 * cos(t3);
        e.z = cost1 * sin(t3);
        p.x = r * e.x;
        p.z = r * e.z;

        normal.set( e.x, e.y, e.z );
        tcoord.set( idivn, jdivn );
        vert.set( p.x, p.y, p.z );

        sphereMesh.addNormal(normal);
        sphereMesh.addTexCoord(tcoord);
        sphereMesh.addVertex(vert);

        e2.x = cost2 * cos(t3);
        e2.z = cost2 * sin(t3);
        p2.x = r * e2.x;
        p2.z = r * e2.z;

        normal.set(e2.x, e2.y, e2.z);
        tcoord.set(idivn, j1divn);
        vert.set(p2.x, p2.y, p2.z);

        sphereMesh.addNormal(normal);
        sphereMesh.addTexCoord(tcoord);
        sphereMesh.addVertex(vert);

        idivn += unodivn;

      }
    }

    return sphereMesh;
  }

};

#endif // PRIMITIVES_H_INCLUDED
