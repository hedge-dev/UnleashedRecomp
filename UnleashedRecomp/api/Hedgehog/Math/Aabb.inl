namespace Hedgehog::Math
{
    inline CVector Hedgehog::Math::CAabb::Center() const
    {
        // return (min + max) / 2;
        Hedgehog::Math::CVector result;
        result.X = (max.X + min.X) * 0.5f;
        result.Y = (max.Y + min.Y) * 0.5f;
        result.Z = 0.5f * (max.Z + min.Z);
        return result;
    }
}
