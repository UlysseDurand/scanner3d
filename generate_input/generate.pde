String nomfichier = "teapot.obj";
int n = 30;

float agrandir = 15;
float decaly = 20;



PShape s;
float alpha;

void setup() {
  size(120, 80, P3D);
  s = loadShape("teapot.obj");
  ortho();
  s.scale(agrandir);
  fill(0);
  s.setFill(color(0));
}

void draw() {
  if (frameCount<=n) {
  alpha = float(frameCount)*PI/n;
  background(255);
  translate(width/2, height/2);
  rotateZ(PI);
  rotateY(alpha);
  shape(s, 0, -decaly);
  saveFrame("generated_inputs/"+nf(frameCount,5)+".png");
  }
  else{
    exit();
  }
}
