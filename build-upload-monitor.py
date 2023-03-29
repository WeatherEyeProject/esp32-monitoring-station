Import("env")

# Multiple actions
env.AddCustomTarget(
    name="Build, Upload, Monitor",
    dependencies=None,
    actions=[
        "pio run --environment az-delivery-devkit-v4",
        "pio run --target upload --environment az-delivery-devkit-v4",
        "pio device monitor --environment az-delivery-devkit-v4"
    ],
    title="Build, Upload, Monitor",
    description="All-in-one solution to your life problems"
)
