# DUAL-MODALITY APPARATUS AND METHOD FOR ASSESSING THE DEGRADATION STATE OF EDIBLE FRYING OIL

**Technical disclosure document**

---

## 1. FIELD OF THE INVENTION

The present invention relates to instrumentation for food safety and quality assurance, and more particularly to a portable apparatus and associated method for determining the degradation state of edible frying oil.

The invention lies at the intersection of dielectric sensing, optical spectroscopy, embedded systems, and food-processing quality control. More specifically, it relates to an apparatus that determines oil degradation by acquiring two physically independent measurements of the same oil sample — a capacitive measurement responsive to the relative permittivity of the oil, and a multi-channel optical measurement responsive to the absorbance of the oil across visible and near-infrared wavelengths — together with a simultaneous temperature measurement used to compensate both signals, and by combining these measurements to produce a cross-validated degradation index.

---

## 2. BACKGROUND OF THE INVENTION

### 2.1 The problem of frying oil degradation

Edible oil used for repeated deep frying undergoes progressive chemical breakdown. At frying temperatures, typically 170 to 190 degrees Celsius, in the presence of atmospheric oxygen, water released from food, and food residues, the oil undergoes hydrolysis, oxidation, polymerisation, and thermal decomposition. These reactions generate a heterogeneous group of degradation products collectively quantified as Total Polar Materials, also referred to as Total Polar Compounds.

The accumulation of polar compounds is accompanied by the formation of free fatty acids, oxidised triglycerides, polymerised species, and coloured compounds. Consumption of heavily degraded frying oil has been associated with adverse health outcomes. Consequently, several jurisdictions impose regulatory limits on the permissible polar-compound content of oil in commercial use.

In India, the Food Safety and Standards Authority of India specifies that used vegetable oil or fat having developed Total Polar Compounds exceeding twenty-five percent shall not be used, with fresh oil separately limited to fifteen percent. Comparable limits are applied elsewhere, generally in the range of twenty-four to twenty-seven percent.

Compliance therefore requires a practical means of determining polar-compound content at the point of use.

### 2.2 Limitations of existing approaches

**Laboratory chromatographic methods.** The recognised reference procedure for determining polar-compound content is silica-gel column chromatography, in which the polar fraction is separated from the non-polar fraction and determined gravimetrically. This method is accurate but requires a laboratory, trained personnel, organic solvents, and a turnaround time of hours. It is unsuitable for use in a commercial kitchen and cannot inform an operational decision about whether to continue frying.

**Commercial dielectric-based handheld meters.** Instruments are commercially available which determine polar-compound content by measuring the dielectric constant of the oil using a capacitive sensing element, converting the measured permittivity to an indicated percentage by means of a factory calibration. Such instruments provide a reading within tens of seconds and are widely used in institutional catering.

These instruments suffer from several limitations. Their acquisition cost is substantial and is prohibitive for small and micro food businesses, street vendors, and individual households, which collectively account for a very large proportion of frying activity, particularly in developing economies. They rely upon a single sensing modality, so an erroneous reading arising from moisture contamination, suspended particulate matter, or sensor fouling cannot be detected by the instrument itself and is presented to the operator as a valid measurement. Published comparisons against the chromatographic reference method report correlation coefficients of approximately 0.87 to 0.90, with systematic discrepancies in particular concentration bands, including a documented tendency to overstate polar-compound content in fresh oils.

**Colorimetric indicators.** Chemical test strips which change colour in response to free fatty acid content are inexpensive but respond to hydrolytic rancidity rather than to total polar compounds. Interpretation is visual and subjective, the strips are consumed by each test, and the correlation with polar-compound content is weak.

**Operator judgement.** In practice, the decision to discard oil is frequently made on the basis of colour, odour, viscosity, or smoking behaviour. These indicators are subjective, vary between operators, are influenced by the food previously fried, and correlate only loosely with the regulated parameter.

### 2.3 The unmet need

There accordingly exists a need for an apparatus which determines the degradation state of frying oil at the point of use, within a short measurement interval, at an acquisition cost accessible to small food businesses, and which is capable of detecting its own erroneous readings rather than presenting them as valid.

The present invention addresses this need. In particular, it addresses the self-validation deficiency of single-modality instruments by acquiring two measurements which respond to different physical consequences of the same underlying degradation process, such that agreement between the two constitutes evidence of measurement validity and disagreement constitutes a detectable fault condition.

---

## 3. SUMMARY OF THE INVENTION

In one aspect, the invention provides an apparatus for assessing the degradation state of edible frying oil, the apparatus comprising:

a probe assembly comprising at least two mutually spaced electrodes formed of a food-grade corrosion-resistant metal, the electrodes being adapted for immersion in a body of the oil such that the oil constitutes the dielectric medium therebetween;

a capacitance-to-digital converter electrically coupled to the electrodes and arranged to determine a capacitance value dependent upon the relative permittivity of the oil;

a multi-channel optical sensor arrangement comprising an illumination source and a plurality of photodetector channels responsive to respective distinct wavelength bands spanning visible and near-infrared regions, arranged to acquire an absorbance profile of the oil;

a temperature sensor arranged in thermal contact with the oil;

a processing unit configured to receive the capacitance value, the absorbance profile, and a temperature value, to apply a temperature correction to at least the capacitance value, to derive from the absorbance profile at least one optical degradation indicator, to combine the temperature-corrected capacitance value and the optical degradation indicator into a degradation index, and to compare the degradation index against a stored threshold;

and an output device arranged to indicate the degradation index and an alert condition when the threshold is exceeded.

In a further aspect, the invention provides a method of assessing the degradation state of edible frying oil, comprising immersing the probe assembly in the oil, acquiring the capacitance value, the absorbance profile, and the temperature value substantially concurrently, applying the temperature correction, deriving the optical degradation indicator, and generating the degradation index; the method further comprising comparing an ordering of the sample derived from the capacitance value against an ordering derived from the optical degradation indicator, and generating a fault indication when the two orderings are inconsistent beyond a predetermined tolerance.

In a further aspect, the invention provides a method of calibrating the apparatus in the absence of a reference polar-compound instrument, comprising measuring the capacitance of a plurality of media of known relative permittivity, fitting a linear relation between measured capacitance and known permittivity to determine a cell constant and a parasitic capacitance offset of the probe assembly, and thereafter expressing subsequent measurements in terms of derived relative permittivity rather than raw capacitance.

The principal advantage conferred by the invention is that two physically independent transduction mechanisms respond to the same degradation process. Capacitive transduction responds to the increase in relative permittivity caused by the accumulation of polar molecular species. Optical transduction responds to the formation of coloured degradation products which absorb preferentially at shorter wavelengths. Because these mechanisms are unrelated, a fault affecting one is unlikely to affect the other in the same direction and by the same magnitude, and the apparatus is therefore able to distinguish a genuine change in oil condition from a measurement artefact.

---

## 4. BRIEF DESCRIPTION OF THE DRAWINGS

**Figure 1** is a perspective view of the assembled apparatus, showing the housing, the probe assembly projecting therefrom, and the display panel.

**Figure 2** is a perspective view of the probe assembly in isolation, showing the spaced electrodes and the temperature sensor mounting.

**Figure 3** is a schematic sectional view of the probe assembly immersed in a body of oil, indicating the electric field region between the electrodes and the position of the temperature sensor.

**Figure 4** is a block diagram of the electronic architecture, showing the processing unit, the capacitance-to-digital converter, the multi-channel optical sensor, the temperature sensor, the display, and the power supply, together with the communication bus interconnecting them.

**Figure 5** is a flow diagram of the measurement and signal-processing method.

**Figure 6** is a graph illustrating the calibration relation between measured capacitance and known relative permittivity for a plurality of reference media, from which the cell constant and parasitic offset are derived.

**Figure 7** is a graph illustrating the variation of measured capacitance with cumulative thermal stress applied to a body of oil.

**Figure 8** is a graph illustrating the variation of an optical ratio indicator, derived from a short-wavelength channel and a near-infrared channel, with the same cumulative thermal stress.

**Figure 9** is a graph illustrating the cross-validation relation between the capacitive indicator and the optical indicator, wherein consistency of ordering between the two indicates measurement validity.

**Figure 10** is a view of the display panel in an alert condition.

---

## 5. DETAILED DESCRIPTION OF THE INVENTION

### 5.1 General arrangement

Referring to Figure 1, the apparatus comprises a housing (10) of moulded or additively manufactured polymeric material, dimensioned to be held in one hand. A probe assembly (20) projects from a first end of the housing. A display (30) is mounted upon a face of the housing. The housing encloses a printed circuit assembly bearing the processing unit and the sensing subsystems, together with a rechargeable electrochemical cell.

The housing is formed in two parts joined along a parting line, permitting access for servicing. In the described embodiment the housing is additively manufactured from polyethylene terephthalate glycol, selected for its dimensional stability and resistance to elevated temperature relative to polylactic acid.

### 5.2 The probe assembly

Referring to Figures 2 and 3, the probe assembly comprises a first electrode (21) and a second electrode (22), each formed as an elongate member of austenitic stainless steel of grade 304 or higher. The electrodes are held in mutually spaced, substantially parallel relation by a dielectric spacer (23) formed integrally with or attached to the housing.

The selection of austenitic stainless steel is material to the operation of the invention. During oil degradation, free fatty acid content increases. Base metals such as copper and brass are attacked by free fatty acids at frying temperatures, and the resulting corrosion products alter the electrode surface condition. Such alteration changes the measured capacitance independently of the permittivity of the oil, introducing a drift which is indistinguishable from genuine degradation. Austenitic stainless steel is substantially inert in this environment and is additionally approved for food contact.

In the described embodiment the electrodes are of threaded form. The thread increases the wetted surface area for a given immersed length, thereby increasing the cell constant and the sensitivity of the apparatus. In an alternative embodiment the electrodes are of smooth cylindrical form, which reduces the retention of oil residues upon the electrode surface between successive measurements and thereby improves reproducibility, at the expense of some sensitivity. In a further alternative embodiment the electrodes are of planar form, arranged as opposed parallel plates.

A temperature sensor (24) is mounted in thermal communication with the oil. In the described embodiment the temperature sensor is a digital semiconductor sensor mounted within or immediately behind a central support member (25) disposed between the electrodes.

It is a feature of the described arrangement that any electrically conductive member disposed within the electric field region between the electrodes, whether electrically connected or electrically floating, modifies the capacitance measured between the electrodes. A floating conductor reduces the effective dielectric path length and thereby increases the measured capacitance. The apparatus accommodates this effect by requiring that the position of such member be mechanically fixed relative to the electrodes, whereby its contribution becomes a constant component of the parasitic capacitance offset and is removed by the calibration procedure described in section 5.6. In an alternative embodiment the support member is formed of a dielectric material, whereby the effect is avoided.

The probe assembly further comprises a depth-limiting formation (26), being a collar, shoulder, or rim-engaging feature which mechanically determines the immersion depth of the electrodes. This feature is material to the operation of the invention. For elongate electrodes, the capacitance between the electrodes varies substantially linearly with wetted length. A variation in immersion depth therefore produces a proportional variation in measured capacitance which may exceed the entire capacitance change associated with the degradation range of interest. Mechanical determination of immersion depth is accordingly preferred over visual or manual determination.

In a preferred embodiment, the conductor connecting the first electrode to the capacitance-to-digital converter is a shielded conductor, the shield being driven by a guard output of the converter at substantially the potential of the sense conductor. This arrangement, being active guarding, substantially reduces the parasitic capacitance contributed by the connecting cable and by nearby objects including the body of the operator, thereby increasing the proportion of the measured capacitance which is attributable to the oil.

### 5.3 The capacitive sensing subsystem

The first electrode is coupled to a first input of a capacitance-to-digital converter (40). The second electrode is coupled to a reference potential of the apparatus. The converter applies an alternating excitation to the first input at a frequency of approximately twenty-five kilohertz and determines the resulting charge transfer, from which the capacitance is derived.

The selection of excitation frequency is material. The relative permittivity of edible oils is substantially independent of frequency over a broad plateau extending from approximately one hundred hertz to approximately five hundred kilohertz, falling thereafter as dielectric dispersion becomes significant. An excitation frequency of twenty-five kilohertz lies within this plateau. Consequently permittivity values reported in the literature at other frequencies within the plateau are applicable to the apparatus, and the dielectric loss of the oil is low at this frequency, favouring a capacitance-only determination.

The converter incorporates a programmable offset capacitance, being a switched capacitor array, which permits the measurement of capacitances substantially exceeding the intrinsic measurement span of the converter. In the described embodiment the intrinsic span is approximately thirty picofarads and the offset extends to approximately one hundred picofarads, giving a total measurable capacitance of approximately one hundred and twelve picofarads. The processing unit adjusts the offset automatically such that the measurement remains within the intrinsic span.

The processing unit acquires a plurality of successive capacitance determinations, in the described embodiment sixty-four, and computes therefrom both a mean value and a dispersion statistic. The dispersion statistic is retained and reported. It serves two purposes: it provides an estimate of measurement uncertainty for the reported value, and an anomalous increase in dispersion indicates a fault condition such as an intermittent electrical connection, mechanical vibration of the probe, or the presence of gas bubbles within the measurement region.

### 5.4 The optical sensing subsystem

The optical sensing subsystem comprises a multi-channel spectral sensor (50) and an associated broadband illumination source. In the described embodiment the sensor comprises three co-packaged semiconductor devices collectively providing eighteen photodetector channels having respective centre wavelengths distributed between approximately four hundred and ten nanometres and approximately nine hundred and forty nanometres, thereby spanning the violet, visible, and near-infrared regions.

The illumination source directs radiation into the oil sample and the photodetector channels receive radiation returned therefrom, whereby the acquired channel values are dependent upon the absorbance of the oil at the respective wavelengths.

As oil degrades, coloured degradation products are formed. The absorbance of these products is strongly wavelength dependent, being pronounced at shorter wavelengths and substantially reduced in the near-infrared region. Accordingly, degradation produces a characteristic differential change across the channel set, in which short-wavelength channels are attenuated substantially while near-infrared channels are attenuated only slightly.

The processing unit derives from the channel values at least one optical degradation indicator. In the described embodiment the indicator is a ratio of a short-wavelength channel value to a near-infrared channel value. The use of a ratio rather than an absolute channel value confers the advantage that variations common to both channels, such as drift in illumination intensity, ageing of the illumination source, variation in the optical path length, and partial fouling of the optical window, are substantially cancelled.

In alternative embodiments the optical degradation indicator comprises a slope fitted across a plurality of channels, a ratio of summed channel groups, or a projection of the channel vector onto a direction determined by prior multivariate analysis.

### 5.5 The processing unit, signal fusion, and output

Referring to Figure 4, the processing unit (60) comprises a microcontroller having an integrated wireless communication facility. The capacitance-to-digital converter and the multi-channel spectral sensor are coupled to the processing unit by a shared two-wire serial bus, the devices being distinguished by respective bus addresses. The temperature sensor is coupled by a single-wire serial interface. The display is coupled to the shared serial bus.

Referring to Figure 5, the processing unit executes the following method upon initiation of a measurement.

The temperature value is acquired. The capacitance is acquired as a plurality of determinations, from which the mean and dispersion are computed. The illumination source is energised and the channel values are acquired, and the illumination source is de-energised.

A temperature correction is applied to the capacitance. The relative permittivity of edible oil exhibits a negative dependence upon temperature. The processing unit normalises the measured value to a reference temperature according to a stored coefficient. This correction is material to the operation of the invention, since the capacitance change attributable to a temperature variation of a few degrees is comparable to the capacitance change attributable to a substantial change in degradation state; absent correction, an uncontrolled temperature variation is indistinguishable from a change in oil condition.

The temperature-corrected capacitance is converted to a derived relative permittivity using the cell constant and parasitic offset established by the calibration procedure of section 5.6. The optical degradation indicator is computed from the channel values.

The derived permittivity and the optical indicator are combined into a degradation index. In the described embodiment the combination is a weighted sum of the two indicators, each first normalised with respect to stored values corresponding to fresh oil and to oil at the discard threshold.

The processing unit further performs a cross-validation operation. The degradation state implied by the derived permittivity is compared with that implied by the optical indicator. Where the two are consistent within a predetermined tolerance, the degradation index is reported. Where the two are inconsistent, a fault indication is generated. Such inconsistency arises characteristically from moisture contamination of the sample, which elevates permittivity substantially while affecting absorbance only slightly; from suspended particulate matter, which affects the optical measurement substantially while affecting permittivity only slightly; and from fouling of the electrodes or of the optical window.

This cross-validation operation constitutes a principal distinction of the invention over single-modality instruments, which possess no means of detecting such conditions.

The degradation index, the derived permittivity, the optical indicator, the temperature, and the dispersion statistic are presented upon the display. Where the degradation index exceeds the stored threshold, an alert condition is indicated. In the described embodiment the threshold corresponds to a polar-compound content of twenty-five percent, in accordance with the applicable regulatory limit, and a secondary advisory threshold is provided at twenty percent. The stored data may be transmitted by the wireless facility to a remote logging system.

### 5.6 Calibration procedure

A method of calibrating the apparatus is provided which does not require access to a reference polar-compound instrument.

In a first stage, the probe assembly is immersed successively in a plurality of media of known relative permittivity, and the capacitance is determined for each. Because the measured capacitance comprises a parasitic component independent of the medium and an active component proportional to the permittivity of the medium, the relation between measured capacitance and known permittivity is substantially linear. Fitting this relation yields a slope, being the cell constant of the probe assembly, and an intercept, being the parasitic capacitance offset.

In a representative determination upon a constructed embodiment, measurement in air, having relative permittivity of substantially unity, yielded a capacitance of 14.4 picofarads, and measurement in fresh vegetable oil, having relative permittivity of substantially three, yielded a capacitance of 36.0 picofarads. Solution of the resulting simultaneous equations gives a cell constant of 10.8 picofarads per unit relative permittivity and a parasitic capacitance offset of 3.6 picofarads. The parasitic offset accordingly constitutes twenty-five percent of the capacitance measured in air.

It is a consequence of this relation that the ratio of the capacitance measured in oil to that measured in air constitutes a figure of merit for the probe assembly. In the absence of parasitic capacitance the ratio would equal the permittivity of the oil, being substantially three. A ratio approaching this value indicates that the measured capacitance is dominated by the oil; a ratio substantially below it indicates that the measurement is dominated by parasitic contributions. The parasitic fraction may be determined from the measured ratio and used to guide improvement of the probe assembly and its connecting cable.

It is a further consequence that where the cell constant is increased in order to improve sensitivity, media of high relative permittivity may exceed the measurement range of the converter. In the representative determination above, media having relative permittivity exceeding approximately ten cannot be measured. In such case the plurality of reference media is selected from among air and a plurality of oils of differing but known permittivity, or alternatively an auxiliary probe assembly of reduced cell constant is provided for the purpose of verifying the linearity of the converter across a wider permittivity range.

In a second stage, a series of oil samples of progressively increasing degradation is prepared by subjecting a body of oil to controlled thermal stress at frying temperature with intermittent introduction of a food load, samples being withdrawn at intervals. Each sample is permitted to attain a common temperature and is measured. The resulting relation between the derived indicators and cumulative thermal stress is stored, as illustrated in Figures 7 and 8.

In a third stage, where absolute expression in terms of polar-compound content is required, the relation is anchored by reference to at least two samples of known polar-compound content, determined by a reference instrument or laboratory procedure. In the absence of such reference, the apparatus reports the degradation index and the derived permittivity, together with an indication that absolute polar-compound content has not been established, and the alert threshold is determined by reference to published permittivity values corresponding to the regulatory limit.

---

## 6. TECHNICAL SPECIFICATIONS

**Capacitive sensing subsystem**

| Parameter | Value |
|---|---|
| Transducer | Capacitance-to-digital converter, four input channels |
| Excitation frequency | Approximately 25 kHz |
| Intrinsic measurement span | Approximately ±15 pF |
| Programmable offset range | To approximately 100 pF |
| Total measurable capacitance | Approximately 112 pF |
| Measurement resolution | Sub-femtofarad at converter |
| Determinations averaged per reading | 64 |
| Cell constant, described embodiment | 10.8 pF per unit relative permittivity |
| Parasitic offset, described embodiment | 3.6 pF |
| Bus interface | Two-wire serial |

**Optical sensing subsystem**

| Parameter | Value |
|---|---|
| Channel count | 18 |
| Spectral coverage | Approximately 410 nm to 940 nm |
| Regions spanned | Violet, visible, near-infrared |
| Illumination | Integrated broadband source |
| Primary derived indicator | Ratio of short-wavelength to near-infrared channel |
| Bus interface | Two-wire serial |

**Temperature sensing subsystem**

| Parameter | Value |
|---|---|
| Transducer | Digital semiconductor temperature sensor |
| Range | Approximately −55 °C to +125 °C |
| Interface | Single-wire serial |
| Function | Compensation of capacitive and optical channels |

**Probe assembly**

| Parameter | Value |
|---|---|
| Electrode material | Austenitic stainless steel, grade 304 or higher |
| Electrode count | Two, mutually spaced |
| Electrode form | Threaded elongate; alternatively smooth or planar |
| Immersion depth control | Mechanical depth-limiting formation |
| Connecting cable | Shielded, with active guard in preferred embodiment |

**Processing and output**

| Parameter | Value |
|---|---|
| Processing unit | Microcontroller with integrated wireless facility |
| Display | Monochrome organic light-emitting diode panel |
| Data output | Delimited text record; wireless transmission optional |
| Power source | Rechargeable lithium-ion cell |
| Housing | Additively manufactured polyethylene terephthalate glycol |

**Alert thresholds**

| Parameter | Value |
|---|---|
| Advisory threshold | Corresponding to 20 percent polar-compound content |
| Discard threshold | Corresponding to 25 percent polar-compound content |
| Basis | Applicable national regulatory limit |

*Note: values stated for cell constant and parasitic offset are as determined upon a constructed embodiment and are dependent upon probe geometry. Values expressed in terms of polar-compound content are subject to establishment of the absolute calibration described in section 5.6, stage three.*

---

## 7. ADVANTAGES OF THE INNOVATION

**7.1 Self-validation through modality independence.** The apparatus acquires two measurements responsive to different physical consequences of the same degradation process. Because the transduction mechanisms are unrelated, a fault affecting one is unlikely to affect the other in the same direction and by the same magnitude. The apparatus is thereby able to detect its own erroneous readings. Single-modality instruments possess no such capability and present erroneous readings to the operator as valid.

**7.2 Detection of specific interference conditions.** Moisture contamination elevates permittivity substantially while affecting absorbance only slightly. Suspended particulate matter affects the optical measurement substantially while affecting permittivity only slightly. Each condition produces a characteristic disagreement between the two modalities and is therefore identifiable rather than merely detectable.

**7.3 Temperature compensation.** Simultaneous temperature measurement permits normalisation of both channels to a reference temperature. Absent such compensation, a temperature variation of a few degrees produces an apparent change in degradation state comparable to a genuine change.

**7.4 Ratiometric optical determination.** Expression of the optical indicator as a ratio between channels causes variations common to both channels, including illumination drift, source ageing, path-length variation, and partial window fouling, to cancel substantially.

**7.5 Reported measurement uncertainty.** The dispersion statistic accompanying each reading permits the operator to distinguish a reliable determination from an unreliable one, and an anomalous dispersion indicates a fault condition. Commercial instruments typically report a single value without uncertainty.

**7.6 Cost accessibility.** The apparatus is constructed from commodity semiconductor sensing devices, a commodity microcontroller, and additively manufactured mechanical components. The resulting acquisition cost is a small fraction of that of commercial instruments, bringing point-of-use determination within reach of small and micro food businesses and individual households.

**7.7 Calibration without reference instrumentation.** The disclosed calibration procedure permits characterisation of the probe assembly and establishment of a relative degradation scale using media of known permittivity, without access to a reference polar-compound instrument. This is material to deployment in settings where such instrumentation is unavailable.

**7.8 Corrosion resistance and material compatibility.** Selection of austenitic stainless steel for the electrodes avoids the progressive drift which arises where base metals are attacked by the free fatty acids generated during degradation, and satisfies food-contact requirements.

**7.9 Mechanical determination of immersion depth.** The depth-limiting formation removes what would otherwise be the dominant source of measurement variability for elongate electrodes.

**7.10 Rapid, non-consumptive determination.** A determination is completed within a short interval, requires no reagents or consumables, and does not destroy the sample.

**7.11 Data retention and traceability.** Records may be retained and transmitted, permitting establishment of an auditable history of oil condition for regulatory or quality-assurance purposes.

---

## 8. INDUSTRIAL APPLICABILITY

The invention is capable of industrial application in the following fields.

**Commercial food preparation.** Restaurants, hotels, institutional catering operations, and quick-service food outlets employing deep frying may determine oil condition at the point of use, enabling oil to be replaced when replacement is warranted rather than according to a fixed schedule. This confers both a food-safety benefit and an economic benefit, since premature replacement represents avoidable cost.

**Micro and small food enterprises.** Street vendors, small eating establishments, and home-based food businesses constitute a substantial proportion of frying activity, particularly in developing economies, and are effectively excluded from existing instrumentation by cost. The invention is directed particularly to this segment.

**Cloud kitchens and delivery-oriented operations.** Such operations frequently conduct high-volume frying with limited on-site supervision, and benefit from an objective determination together with a retained record.

**Regulatory inspection and enforcement.** Food-safety authorities may employ the apparatus for field screening. The self-validation capability is of particular value in this application, since a reading which the instrument itself identifies as suspect may be referred for laboratory confirmation rather than acted upon.

**Industrial food manufacture.** Producers of fried snack foods and prepared foods may employ the apparatus, or a fixed variant thereof, for in-process monitoring of frying media.

**Used cooking oil collection and processing.** Undertakings engaged in the collection of used cooking oil for conversion to biodiesel or other products may employ the apparatus to grade incoming material.

**Institutional catering in sensitive settings.** School, hospital, and workplace canteens serving vulnerable populations may employ the apparatus to demonstrate compliance.

**Domestic use.** Households conducting repeated frying may employ the apparatus to determine when replacement is warranted.

**Education and research.** The apparatus, together with its data-recording facility, is applicable as an instrument for instruction and investigation in food science and instrumentation.

---

## 9. POTENTIAL VARIATIONS AND FUTURE ENHANCEMENTS

**9.1 Electrode geometry.** The electrodes may be of threaded, smooth cylindrical, planar, coaxial, or interdigitated form. Coaxial arrangement, in which an inner electrode is surrounded by an outer electrode, confers inherent shielding against external coupling. Interdigitated planar arrangement permits a substantially flat probe face and a well-defined sensing volume.

**9.2 Dielectric support member.** The central support member may be formed of a dielectric material, whereby its contribution to the measured capacitance is eliminated rather than merely rendered constant.

**9.3 Differential and multi-channel capacitive measurement.** The converter provides a plurality of input channels. A second electrode pair may be provided, disposed outside the oil or within a sealed reference medium, and a differential determination taken, whereby common-mode drift arising from temperature and ageing is substantially cancelled.

**9.4 Multivariate treatment of the spectral data.** Rather than a single channel ratio, the full eighteen-channel vector may be subjected to multivariate analysis, including principal component analysis or partial least squares regression, to derive an indicator of improved discrimination. A classifier may further be trained to distinguish oil type, in which case the appropriate calibration relation may be selected automatically.

**9.5 Oil-type identification and automatic calibration selection.** Different oils exhibit different fresh-state permittivity and different spectral signatures, the former varying systematically with degree of unsaturation. The apparatus may identify the oil type from its fresh-state signature and select the corresponding calibration relation.

**9.6 Machine-learned degradation estimation.** Where a body of paired measurement and reference data is accumulated, a regression model may be trained to estimate polar-compound content from the combined sensor vector, potentially exceeding the accuracy attainable from either modality individually.

**9.7 Additional sensing modalities.** Further modalities may be incorporated to extend the cross-validation principle, including viscosity determination, which correlates strongly with polar-compound content; electrical conductivity, subject to the qualification that the reported correlation with polar-compound content is inconsistent between studies; and fluorescence, which responds to particular oxidation products.

**9.8 High-temperature and in-situ operation.** The probe assembly may be adapted for continuous immersion within a fryer vessel at frying temperature, whereby degradation is monitored continuously rather than by discrete sampling. Such adaptation requires elevated-temperature materials, an extended temperature-compensation range, and appropriate thermal isolation of the electronics.

**9.9 Fixed installation and fryer integration.** The apparatus may be integrated into a fryer as an original or retrofitted component, and may be arranged to inhibit operation or to generate an alarm upon exceedance of the discard threshold.

**9.10 Networked monitoring and fleet management.** A plurality of apparatus may report to a common remote system, permitting an operator of multiple premises to monitor oil condition across a fleet, to identify anomalous premises, and to generate compliance documentation automatically.

**9.11 Mobile application interface.** A wireless-connected mobile application may present readings, retain history, and provide guidance, permitting simplification or omission of the on-device display.

**9.12 Predictive estimation of remaining service life.** From the trajectory of successive determinations, the apparatus may estimate the interval remaining before the discard threshold is attained, permitting replacement to be scheduled in advance.

**9.13 Self-cleaning and anti-fouling provisions.** The probe may incorporate a wiper, a hydrophobic or oleophobic surface treatment, or an ultrasonic transducer to reduce the retention of residues, particularly relevant where threaded electrodes are employed.

**9.14 Compensation for moisture and particulate content.** Dedicated determination of moisture content, whether by a further capacitive channel at a differing excitation frequency or by a near-infrared absorption band characteristic of water, would permit correction rather than mere detection of moisture interference.

**9.15 Adaptation to further media.** The underlying principle is applicable to the assessment of other media in which degradation alters both permittivity and optical absorbance, including lubricating oils, transformer insulating oils, and hydraulic fluids.

---

## 10. ABSTRACT

An apparatus and method for assessing the degradation state of edible frying oil are disclosed. The apparatus comprises a probe assembly having mutually spaced electrodes of austenitic stainless steel adapted for immersion in the oil, a capacitance-to-digital converter coupled thereto for determining a capacitance dependent upon the relative permittivity of the oil, a multi-channel optical sensor providing eighteen photodetector channels spanning visible and near-infrared wavelengths for acquiring an absorbance profile of the oil, a temperature sensor in thermal communication with the oil, a processing unit, and a display.

The processing unit applies a temperature correction to the capacitance, converts the corrected capacitance to a derived relative permittivity using a cell constant and parasitic offset previously established by measurement of media of known permittivity, derives an optical degradation indicator as a ratio between a short-wavelength channel and a near-infrared channel, and combines the two indicators into a degradation index which is compared against a stored threshold corresponding to a regulatory limit on polar-compound content.

The two indicators respond to physically independent consequences of oil degradation, namely the increase in relative permittivity caused by accumulation of polar species and the increase in short-wavelength absorbance caused by formation of coloured degradation products. The processing unit compares the degradation state implied by each and generates a fault indication upon inconsistency, whereby interference conditions including moisture contamination, suspended particulate matter, and electrode or window fouling are rendered detectable and identifiable. The apparatus thereby provides a self-validating determination, in contrast to single-modality instruments which present erroneous readings as valid, at an acquisition cost accessible to small food enterprises.

---

*This document is a technical disclosure prepared for descriptive purposes. It is not a legal filing and does not constitute legal advice. Where protection of the subject matter is sought, a qualified patent agent or attorney should be consulted, and the disclosure should be reviewed for novelty against the prior art before any public disclosure occurs, since publication prior to filing may prejudice patentability in certain jurisdictions.*
