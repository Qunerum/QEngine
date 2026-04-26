using QEngine.GUI;

// ReSharper disable All

namespace QEngine.Geometry
{
    public struct Model
    {
        public Vector3[] verts;
        public uint[] inds;
        public Color color;
        public Model(Vector3[] verts, uint[] inds, Color color) { this.verts = verts; this.inds = inds; this.color = color; }
    }
}