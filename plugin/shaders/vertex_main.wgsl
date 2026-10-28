//=================================================
//Vertex Shader
//=================================================
fn projectPerspective(worldPos: vec3f) -> vec4f {
    return /*u.viewProjMatrix * */ vec4f(worldPos, 1.0);
}

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
    var out      : VertexOutput;
    var position = in.position;

    switch u.materialId {
        case MAT_PLANE:  { out.clipPos = vertexPlane         (&position); }
        case MAT_SPHERE: { out.clipPos = vertexSphere        (&position); }
        default:         { out.clipPos = projectPerspective  (position);  }
    }

    out.color    = in.color;
    out.worldPos = position;
    out.normal   = in.normal;

    return out;
}

