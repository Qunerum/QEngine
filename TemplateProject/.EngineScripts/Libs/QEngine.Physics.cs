using System;

using QEngine.GUI;

using QEngine.Dev.Renderer;
// ReSharper disable All

namespace QEngine.Physics
{
    /// <summary> Global system that handles collision detection and resolution for all objects in the scene. </summary>
    public static class Physics
    {
        /// <summary> 
        /// Processes physics for the current scene. 
        /// Runs multiple iterations to ensure stability for stacked objects.
        /// </summary>
        public static void Step()
        {
            var scene = SceneManager.actualScene;
            if (scene == null) return;
            var objects = scene.GetObjects();

            int iterations = 8;
            for (int it = 0; it < iterations; it++)
            {
                for (int i = 0; i < objects.Length; i++)
                {
                    if (!objects[i].TryGetComponent(out BoxCollider2D col1)) continue;
                    objects[i].TryGetComponent(out Rigidbody2D rb1);

                    for (int j = i + 1; j < objects.Length; j++)
                    {
                        if (!objects[j].TryGetComponent(out BoxCollider2D col2)) continue;
                        objects[j].TryGetComponent(out Rigidbody2D rb2);

                        bool rb1CanMove = rb1 != null && !rb1.isStatic;
                        bool rb2CanMove = rb2 != null && !rb2.isStatic;
                        if (rb1CanMove || rb2CanMove) Resolve(rb1, col1, rb2, col2);
                    }
                }
            }
        }

        static void Resolve(Rigidbody2D? rb1, BoxCollider2D? b1, Rigidbody2D? rb2, BoxCollider2D? b2)
        {
            if (rb1 == null || rb2 == null || b1 == null || b2 == null) return;
            AABB a = b1.Bounds, b = b2.Bounds;
            if (a.Overlaps(b))
            {
                float overlapX = Math.Min(a.Max.x, b.Max.x) - Math.Max(a.Min.x, b.Min.x),
                    overlapY = Math.Min(a.Max.y, b.Max.y) - Math.Max(a.Min.y, b.Min.y);

                bool canMove1 = rb1 != null && !rb1.isStatic, 
                    canMove2 = rb2 != null && !rb2.isStatic;
                if (overlapX < overlapY)
                {
                    float dir = a.Min.x < b.Min.x ? -1 : 1,
                        move = canMove1 && canMove2 ? overlapX / 2f : overlapX;
                    if (canMove1 && b1.transform != null) b1.transform.position.x += move * dir;
                    if (canMove2 && b2.transform != null) b2.transform.position.x -= move * dir;
                    if (canMove1 && canMove2)
                    {
                        float combinedVel = (rb1!.velocity.x + rb2!.velocity.x) / 2f;
                        rb1.velocity.x = rb2.velocity.x = combinedVel;
                    }
                    else if (canMove1) rb1!.velocity.x = 0;
                    else if (canMove2) rb2!.velocity.x = 0;
                }
                else
                {
                    float dir = a.Min.y < b.Min.y ? -1 : 1,
                        move = (canMove1 && canMove2) ? overlapY / 2f : overlapY;
                    if (canMove1 && b1.transform != null) b1.transform.position.y += move * dir;
                    if (canMove2 && b2.transform != null) b2.transform.position.y -= move * dir;
                    if (dir > 0) { if (canMove1 && rb1!.velocity.y < 0) rb1.velocity.y = 0; }
                    else { if (canMove2 && rb2!.velocity.y < 0) rb2.velocity.y = 0; }
                }
            }
        }
        /// <summary> Casts a ray against all colliders in the scene and returns the closest hit. </summary>
        /// <param name="origin">The starting point of the ray in world space.</param>
        /// <param name="direction">The direction vector of the ray.</param>
        /// <param name="maxDistance">Maximum distance the ray can travel.</param>
        /// <param name="hit">Output structure containing hit details if successful.</param>
        /// <param name="ignore">Optional GameObject to ignore during the cast (e.g., the shooter).</param>
        /// <param name="showRay">If true, draws a debug line in the scene (Green on hit, Red on miss).</param>
        /// <returns>True if the ray intersected with a collider; otherwise, false.</returns>
        public static bool Raycast(Vector2 origin, Vector2 direction, float maxDistance, out RaycastHit hit, GameObject? ignore = null, bool showRay = false)
        {
            hit = new RaycastHit();
            var scene = SceneManager.actualScene;
            if (scene == null) return false;

            float len = MathF.Sqrt(direction.x * direction.x + direction.y * direction.y);
            if (len == 0) return false;
            Vector2 dir = new Vector2(direction.x / len, direction.y / len);

            var objects = scene.GetObjects();
            float closestDist = maxDistance;
            bool found = false;
            foreach (var obj in objects)
            {
                if (obj == ignore) continue;
                if (!obj.TryGetComponent(out BoxCollider2D col)) continue;
                AABB b = col.Bounds;
                float t1 = (b.Min.x - origin.x) / dir.x, t2 = (b.Max.x - origin.x) / dir.x,
                    t3 = (b.Min.y - origin.y) / dir.y, t4 = (b.Max.y - origin.y) / dir.y,
                    tmin = Math.Max(Math.Min(t1, t2), Math.Min(t3, t4)), tmax = Math.Min(Math.Max(t1, t2), Math.Max(t3, t4));

                if (tmax < 0 || tmin > tmax) continue;
                if (tmin > 0.1f && tmin < closestDist)
                {
                    closestDist = tmin;
                    hit.gameObject = obj;
                    hit.point = new Vector2(origin.x + dir.x * tmin, origin.y + dir.y * tmin);
                    found = true;
                } 
            }
            if (!found) { hit.point = origin + direction * maxDistance; }
            if (showRay) QRenderer.DrawLine(origin, hit.point, 3, (found ? Color.Green : Color.Red).to01());
            return found;
        }
    }

    #region Structs
    /// <summary>  Represents an Axis-Aligned Bounding Box (AABB) used for 2D collision detection. </summary>
    public struct AABB
    {
        /// <summary> The minimum corner of the bounding box (bottom-left). </summary>
        public Vector2 Min;
        /// <summary> The maximum corner of the bounding box (top-right). </summary>
        public Vector2 Max;

        /// <summary> Initializes a new AABB based on world position and size. </summary>
        /// <param name="center">The world position (center) of the box.</param>
        /// <param name="size">The full width and height of the box.</param>
        public AABB(Vector2 center, Vector2 size)
        {
            Vector2 half = new Vector2(size.x / 2f, size.y / 2f);
            Min = center - half;
            Max = center + half;
        }

        /// <summary> Checks if this AABB overlaps with another. </summary>
        public bool Overlaps(AABB other)
        {
            return (Min.x <= other.Max.x && Max.x >= other.Min.x) &&
                   (Min.y <= other.Max.y && Max.y >= other.Min.y);
        }
    }
    /// <summary> Contains information about a successful Raycast hit. </summary>
    public struct RaycastHit
    {
        /// <summary> The GameObject that was hit by the ray. </summary>
        public GameObject gameObject;
        /// <summary> The exact world-space point where the intersection occurred. </summary>
        public Vector2 point;
    }
    #endregion
    
    #region Components
    /// <summary> 
    /// Enables a GameObject to react to gravity, forces, and friction. 
    /// Works in conjunction with <see cref="BoxCollider2D"/> for physical interactions.
    /// </summary>
    public class Rigidbody2D : Component
    {
        /// <summary> The current velocity of the object in world units per second. </summary>
        public Vector2 velocity = new(0, 0);
        /// <summary> Multiplier for the global gravity force. </summary>
        public float gravityScale = 1.0f;
        /// <summary> The mass of the object. Higher mass results in less acceleration from forces. </summary>
        public float mass = 1.0f;
        /// <summary> 
        /// Linear air/surface resistance. Reduces horizontal velocity over time. 
        /// Values between 5.0 and 15.0 are recommended for most games.
        /// </summary>
        public float drag = 10.0f;
        /// <summary> If true, the object is immovable and ignored by gravity, acting as a static obstacle. </summary>
        public bool isStatic = false;
        /// <summary> If true, the collider's boundaries will be drawn in the scene. </summary>
        public bool showColliders = false;
        /// <summary> The global gravity constant used by all Rigidbody2D components. </summary>
        static readonly Vector2 Gravity = new(0, -9.81f * 50f);
        
        /// <summary> Applies an instantaneous force, modified by the object's mass. </summary>
        /// <param name="force">The direction and magnitude of the force to apply.</param>
        public void AddForce(Vector2 force)
        {
            if (isStatic || mass <= 0) return;
            velocity += force * 100 / mass;
        }
        /// <summary> Performs basic Euler integration for movement. </summary>
        public override void FixedUpdate()
        {
            if (isStatic || transform == null) return;
            velocity.x *= 1f / (1f + drag * Time.fixedDeltaTime);
            velocity += Gravity * gravityScale * Time.fixedDeltaTime;
            transform.position += velocity * Time.fixedDeltaTime;
        }
    }
    
    /// <summary> Defines a rectangular physical boundary for a GameObject. </summary>
    public class BoxCollider2D : Component, IRenderable
    {
        /// <summary> The size of the collider in world units. </summary>
        public Vector2 size = new(100, 100);
        /// <summary> Local offset relative to the GameObject's position. </summary>
        public Vector2 offset = new(0, 0);

        /// <summary> Returns the calculated world-space AABB of this collider. </summary>
        public AABB Bounds => transform == null ? new() : new AABB(transform.position + offset, size);

        public bool isUI { get; set; } = false;
        public void Draw() { if (gameObject == null || transform == null) return; 
            if (gameObject.TryGetComponent(out Rigidbody2D rb) && rb.showColliders) 
                QRenderer.DrawWireBox(transform.position + offset, size, 2, new Color(100, 255, 100, 200).to01()); }
    }
    
    #endregion 
}
