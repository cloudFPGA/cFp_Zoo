#### MCEuropeanEngine 



The Xilinx ® Vitis™ Quantitative Finance library is a fundamental library aimed at providing a comprehensive FPGA acceleration library for quantitative finance. It is a free/open-source for a variety of real use cases, such as modeling, trading, evaluation, and risk management.

The Vitis Quantitative Finance library provides comprehensive tools from the bottom up for quantitative finance. It includes the lowest level modules and functions, the pre-defined middle kernel level, and the third level as pure software APIs working with pre-defined hardware overlays.

This is the implementation of a common option pricing engine (L2 level), specifically the European Option pricing engine.

In Quantitative finance, option pricing is to calculate the perineum for purchasing for selling options.

In options trading, option pricing models provide the finance professional a fair value to adjust their trading strategies and portfolios.

The value of the options are calculated based on current value of the underlying asset, volatility of the underlying asset, dividends paid on the underlying asset, strike price of the option, time to expiration on the option and riskless interest rate.

More info for the `MCEuropeanEngine IP`: https://xilinx.github.io/Vitis_Libraries/quantitative_finance/2020.1/rst_L2/namespace_xf_fintech.html#cid-xf-fintech-mceuropeanengine

The European option pricing engine estimates the value of option based on Monte Carlo simulation method and Black-Scholes model.

As the below figure shows, the Monte Carlo simulation process is accelerated by multiple Monte Carlo model (MCM) units operating in dataflow, and each sub-module in MCM working in pipeline.

![Oveview of Vitis Quantitative Finance MCEuropeanEngine workflow](../../../../doc/cFp_Vitis_mce.png)

All sub-modules in MCM are connected by HLS stream.

The latency of pricing engines based on Monte Carlo simulation is:

`Number of cycles = requiredSamples * timeSteps / MCM:`

The `MCEuropeanEngine IP` is wrapped in an appropriate cFp_Vitis ROLE.