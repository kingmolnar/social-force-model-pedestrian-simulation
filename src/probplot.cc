

struct {
  double length;
  double width;
  double ratio;
  double framewidth;
  double vmax;
  double color;
} ArrowParameters;

void ArrowLength(double l, double r, double max) {
  ArrowParameters.length = l;
  ArrowParameters.ratio = r;
  ArrowParameters.width = ratio*l;
  ArrowParameters.vmax = max;
}

void ArrowFrameWidth(double f) {ArrowParameters.framewidth = f;}

void ArrowDraw(Vector X, Vector V, int color_index) {
  const int num_points = 7; 
  float x[num_points];
  float y[num_points];
  float r = ArrowParameters.ratio;
  float laxis = V.length()/ArrowParameters.max;
  laxis = (laxis<r?r:laxis)*ArrowParameters.length;
  float ex = V.x/V.length();
  float ey = V.y/V.length();
  float sx = -ey;
  float sy = ex;
  float lw = laxis-ArrowParameters.width;
  float w2 = ArrowParameters.width/2,0;
  float w3 = w2*r;
  float px = X.x + ex*lw;
  float py = X.y + ey*lw;
  
  x[0] = X.x + ex*laxis;
  y[0] = X.y + ey*laxis;
  x[1] = px + sx*w2;
  y[1] = py + sy*w2;
  x[2] = px + sx*w3;
  y[2] = py + sy*w3;
  x[3] = X.x + sx*w3;
  y[3] = X.y + sy*w3;
  x[4] = X.x - sx*w3;
  y[4] = X.y - sy*w3;
  x[5] = px - sx*w3;
  y[5] = py - sy*w3;
  x[6] = px - sx*w2;
  y[6] = py - sy*w2;
  
  // void XuPolygonDraw (float x_array[num_points], float y_array[num_points],
  //                     int num_points, int color_index, double frame_width)
  XuPolygonDraw (x, y, num_points, color_index, ArrowParameters.frame_width);
}                         


