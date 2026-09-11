# Campaign difficulty diagnostics

The authoritative campaign validator answers whether a level is mechanically valid and assigns its deterministic difficulty score. The companion diagnostic explains which geometry is responsible for that result.

Run:

```text
python3 Development/AI_Validation/explain_campaign.py --campaign
```

For a single level:

```text
python3 Development/AI_Validation/explain_campaign.py --level Game/Assets/Levels/nivel_24.lvl
```

The diagnostic reports, per reachable transition:

- robustness under charge and launch-position noise;
- horizontal landing margin;
- available launch-position margin;
- charge margin around the selected jump;
- horizontal and vertical displacement;
- target platform width;
- the transition that dominates each risk component.

The final section identifies the strongest weighted bottleneck and gives a geometry-oriented adjustment hint. This is feedback for level authors, not a second validator: it reuses the authoritative parser and physics simulator and does not change gameplay physics, reachability rules or difficulty thresholds.

When calibrating campaign content, adjust geometry in response to the diagnostic rather than weakening the validator. For example, a low landing margin points toward target width/displacement; low robustness points toward sensitivity to launch/charge error; low launch margin points toward limited preparation space.
