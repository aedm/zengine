#include "util.h"
#include "c4dloader.h"
#include <c4d.h>
#include <default_alien_overloads.h>

using namespace melange;

void GetWriterInfo(Int32 &id, String &appname) {
  id = 0x0da5b001;
  appname = "ZenGarden";
}

Mesh* LoadPolyObject(PolygonObject* polyObject) {
  struct Triplet { UINT c, n, t; };
  vector<Triplet> triplets;

  Int32 pointCount = polyObject->GetPointCount();
  Int32 polygonCount = polyObject->GetPolygonCount();
  const Vector* points = polyObject->GetPointR();
  const CPolygon* polys = polyObject->GetPolygonR();

  // access the polygons and points
  for (Int32 i = 0; i < polygonCount; i++) {
    // get the 3 / 4 points of the triangle / quadrangle
    Vector pointA = points[polys[i].a];
    Vector pointB = points[polys[i].b];
    Vector pointC = points[polys[i].c];
    // only if the index of 3rd and 4th point is different we have a quad, if not we have a triangle
    if (polys[i].c != polys[i].d) {
      Vector pointD = points[polys[i].d];
    }
  }
  
  Mesh* mesh = TheResourceManager->CreateMesh();
  mesh->AllocateVertices(VertexPosUVNorm::format, triplets.size());
  //mesh->UploadVertices(&vertices[0]);
  return mesh;
}

OWNERSHIP Mesh* Util::LoadC4DMesh(const QString& fileName) {
  NOT_IMPLEMENTED;

  BaseDocument* c4dDoc = LoadDocument(Filename(fileName.toStdString().c_str()), SCENEFILTER_OBJECTS);
  BaseObject* object = c4dDoc->GetFirstObject();
  
  while (object) {
    // get basic data
    String name = object->GetName();
    Int32 type = object->GetType(); // Ocube, Ospline, Opolygon, Olight, Ocamera, ...
    melange::Matrix ml = object->GetMl(); // local matrix
    melange::Matrix mg = object->GetMg(); // global matrix
                                 // depending on the type, get specific data for the current object
    GeData data; // universal data type, can handle all data formats (Float, Int32, Bool, Vector, ...)
    if (type == Ocamera) {
      object->GetParameter(CAMERAOBJECT_FOV, data);
      Float fov = data.GetFloat();
    }
    // some objects have their own type definition
    else if (type == Opolygon) {
      PolygonObject* polyObject = (PolygonObject*)object;
      return LoadPolyObject(polyObject);
    }
    
    // TODO: do the children
    //BaseObject* children = object->GetDown();
    
    // get the next object
    object = object->GetNext();
  }

  BaseDocument::Free(c4dDoc);
  return nullptr;
}
