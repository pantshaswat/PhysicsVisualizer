#include "config.h"
#include <vector>

class TriangleMesh {
public:
  TriangleMesh();
  void draw();
  ~TriangleMesh();

private:
  unsigned int EBO, VAO, vertex_count;
  std::vector<unsigned int> VBOs;
};
