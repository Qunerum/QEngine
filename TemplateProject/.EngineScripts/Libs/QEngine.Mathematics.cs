using System;
using QEngine.GUI;

namespace QEngine.Mathematics
{
    public static class QMath
    {
        public static float Remap(float value, float inMin, float inMax, float outMin, float outMax) => (value - inMin) / (inMax - inMin) * (outMax - outMin) + outMin;
        public static float Round(float value, int decimals) => MathF.Round(value, decimals);
        public static Vector2 Round(Vector2 value, int decimals) => new(Round(value.x, decimals), Round(value.y, decimals));
        
        public static Vector2 AngleToVector(float angleDeg)
        {
            float rad = angleDeg * MathF.PI / 180f, x = MathF.Cos(rad), y = MathF.Sin(rad);
            return new (x, y);
        }
        
        #region Lerps
        public static float Lerp(float a, float b, float t, int decimals = 4) => 
            Round(a + (b - a) * Math.Clamp(t, 0, 1), decimals);
        public static Vector2 Lerp(Vector2 a, Vector2 b, float t, int decimals = 4) => 
            Round(a + (b - a) * Math.Clamp(t, 0, 1), decimals);
        public static Color Lerp(Color a, Color b, float t)
        => new((byte)Math.Clamp(a.r + (b.r - a.r) * Math.Clamp(t, 0, 1), 0, 255),
                (byte)Math.Clamp(a.g + (b.g - a.g) * Math.Clamp(t, 0, 1), 0, 255),
                (byte)Math.Clamp(a.b + (b.b - a.b) * Math.Clamp(t, 0, 1), 0, 255),
                (byte)Math.Clamp(a.a + (b.a - a.a) * Math.Clamp(t, 0, 1), 0, 255));
        #endregion
    }
}
