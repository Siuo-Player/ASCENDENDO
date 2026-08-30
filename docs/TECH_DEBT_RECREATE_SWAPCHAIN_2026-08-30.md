# Technical debt — swapchain recovery

The current renderer uses a fail-closed policy when swapchain recreation fails after the device-idle boundary. This is now characterized by integration coverage.

The characterization does not introduce a retry or restartable path. Deferred work includes restartable or transactional recreation, but it should not be implemented until a product/runtime requirement establishes that terminal rendering failure is insufficient.
