# Technical debt — swapchain recovery

The current renderer uses a fail-closed policy when swapchain recreation fails after the device-idle boundary. This is now characterized by integration coverage.

Deferred work: restartable or transactional recreation. Do not implement until a product/runtime requirement establishes that terminal rendering failure is insufficient.
