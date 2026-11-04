#pragma once

namespace arm {
    constexpr float baseHeight = 0.70f;
    constexpr float baseRadius = 1.10f;
    constexpr float dicGap = 0.001f;
    
    constexpr float shoulderRadius = 0.24f;
    constexpr float upperArmLength = 2.80f;
    constexpr float upperArmRadius = 0.22f;
    
    constexpr float elbowRadius = 0.20f;
    constexpr float forearmLength = 2.25f;
    constexpr float forearmRadius = 0.17f;

    constexpr float wristRadius = 0.17f;

    // After scaling the palm to 0.82 along Y, we translate it
    // by palmLength * 0.5f (0.41) so it starts at the wrist and ends at y = 0.82.
    constexpr float palmLength = 0.82f;     // y-axis
    constexpr float palmThinkness = 0.36f;  // x-axis
    constexpr float palmHeight = 0.68f;     // z-axis   The stem sits at +-0.34 (half of PalmHeight)

    constexpr float stemLength = 0.58f;
    constexpr float stemRadius = 0.045f;

    constexpr float fingerJointRadius = 0.06f;
    constexpr float fingerLength = 0.80f; 
    constexpr float fingerThickness = 0.085f;
    constexpr float fingerHeight = 0.07f;
}