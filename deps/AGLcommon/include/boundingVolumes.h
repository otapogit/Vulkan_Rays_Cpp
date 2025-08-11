#pragma once

// dependencies (glm)
#include <glm/glm.hpp>
// STL
#include <algorithm>
#include <fstream>
#include <limits>    // for FLT_MAX
#include <vector>    // for vector

namespace autis {
	/*
	Clase que contiene una caja de inclusión alineada a los ejes (las coordenadas mínimas y máximas de los
	vértices de un modelo)
	*/

	struct BoundingBox {
		/**
		Constructor por defecto. La caja de inclusión no es válida.
		*/
#undef min
#undef max

		BoundingBox() :
			min(glm::vec3(std::numeric_limits<float>::max())),
			max(glm::vec3(std::numeric_limits<float>::lowest()))
		{};
		/**
		Constructor mediante dos esquinas opuestas.
		Da igual las esquinas que se proporcionen. Se calculará automáticamente
		las esquinas mínima y máximas
		\param p primera esquina
		\param q esquina opuesta a q
		*/
		BoundingBox(const glm::vec3& p, const glm::vec3& q);
		/**
		\return la dimensión máxima de la caja de inclusión en los ejes principales
		*/
		float getMaxDimension() const {
			return std::max({ max.x - min.x, max.y - min.y, max.z - min.z });
		}
		/**
		\return la coordenada del centro de la caja
		*/
		glm::vec3 getCenter() const { return (max + min) / 2.0f; }
		/**
		Coordenadas de la esquina inferior izquierda posterior y de la superior derecha anterior
		*/
		glm::vec3 min, max;
		/**
		\return si la caja es válida
		*/
		bool isValid() const {
			return min.x != std::numeric_limits<float>::max() &&
				max.x != std::numeric_limits<float>::lowest();
		}
		/**
		Agranda la caja de inclusión para abarcar 'other'
		\param other el volumen de inclusión a abarcar
		*/
		void grow(const BoundingBox& other);
		/**
		Agranda la caja de inclusión para abarcar 'other'
		\param other el volumen de inclusión a abarcar
		*/
		void grow(const glm::vec3& other);
		/**
		Invalida la caja de inclusión
		*/
		void reset() {
			min = glm::vec3(std::numeric_limits<float>::max());
			max = glm::vec3(std::numeric_limits<float>::lowest());
		}
		/**
		Aplica la transformación a la caja de inclusión
		*/
		void transform(const glm::mat4& xform);
		/**
		Devuelve las 8 esquinas del volumen de inclusión. Empieza en la cara superior
		en sentido antihorario, y luego las correspondientes en sentido horario de la
		cara inferior. Empieza por el vértice superior izquierdo anterior.
		*/
		std::vector<glm::vec4> getVertices() const;

		//! \return los lados de la caja en cada dirección
		glm::vec3 extents() const {
			return max - min;
		}

		//! \return el volumen de la caja
		float volume() const {
			if (!isValid()) return 0.0f;
			auto e = extents();
			return e.x * e.y * e.z;
		}
	};


	BoundingBox combineBoundingBoxes(const BoundingBox& a, const BoundingBox& b);

	enum class BoundingBoxCompare {
		Contains, IsContained, Intersect, Equal, Separated
	};

	/**
	 * @brief Returns the relationship between two bounding boxes
	 * @param a 
	 * @param b 
	 * @return if a Contains b, if a IsContained in b, if they intersect, if they are equal or 
	 * if they are separated
	*/
	BoundingBoxCompare compare(const BoundingBox& a, const BoundingBox& b);

	bool inside(const BoundingBox& a, const glm::vec3& p);

	BoundingBox computeBoundingBox(const float* v, uint32_t ncomponents, size_t n);
	std::ostream& operator<<(std::ostream& os, const BoundingBox& s);
	inline bool operator==(const BoundingBox& lhs, const BoundingBox& rhs) {
		return lhs.min == rhs.min && rhs.max == lhs.max;
	}
	inline BoundingBox operator+(BoundingBox lhs, const glm::vec3& off) {
		lhs.min += off;
		lhs.max += off;
		return lhs;
	}


	template<typename C>
	struct BoundingBoxGLM {
		BoundingBoxGLM() { reset(); };
		BoundingBoxGLM(const C& c1, const C& c2) : min{ glm::min(c1, c2) }, max{ glm::max(c1, c2) } {};

		void reset() {
			min = C(std::numeric_limits<typename C::value_type>::max());
			max = C(std::numeric_limits<typename C::value_type>::lowest());
		}

		C center() const {
			return (min + max) * static_cast<C::value_type>(0.5);
		}

		void grow(const BoundingBoxGLM<C>& other) {
			if (other.isValid()) {
				min = glm::min(min, other.min);
				max = glm::max(max, other.max);
			}
		}
		void grow(const C& p) {
			grow(BoundingBoxGLM<C>{p, p});
		}

		bool isValid() const {
			return min.x <= max.x && min.y <= max.y;
		}
		typename C::value_type getMaxDimension() const {
			if (!isValid()) return 0;
			auto diff = max - min;
			C::value_type maxC = diff[0];
			for (C::length_type i = 1; i < diff.length(); i++) {
				if (maxC < diff[i])
					maxC = diff[i];
			}
			return maxC;
		}

		int getMaxDimensionIndex() const {
			if (!isValid()) return -1;
			auto diff = max - min;
			int maxI = 0;
			for (C::length_type i = 1; i < diff.length(); i++) {
				if (diff[maxI] < diff[i])
					maxI = i;
			}
			return maxI;
		}

		typename C::value_type surfaceArea() const {
			if (!isValid())
				return static_cast<C::value_type>(0.0);
			auto extent = max - min;
			auto area = extent.x * extent.y;
			if constexpr (C::length() > 2) area += extent.x * extent.z + extent.y * extent.z;
			return static_cast<C::value_type>(2.0) * area;
		}

		C min;
		float _tmp1;
		C max;
		float _tmp2;
	};
	template<typename C>
	bool operator==(const BoundingBoxGLM<C>& lhs, const BoundingBoxGLM<C> &rhs) {
		return lhs.min == rhs.min && rhs.max == lhs.max;
	}

	template<typename T>
	BoundingBoxGLM<typename T::value_type> computeBoundingBoxGLM(const T& pts) {
		BoundingBoxGLM<T::value_type> res;
		for (const auto &p : pts) {
			res.grow(p);
		}
		return res;
	}

	template<typename T>
	BoundingBoxGLM<typename T> computeBoundingBoxGLM(const T* vtcs, const uint32_t *inds, uint32_t first, uint32_t last) {
		BoundingBoxGLM<T> res;
		for (auto i = first; i <= last; i++) {
			for (auto j = 0; j < 3; j++) {
				const auto& v = vtcs[inds[i * 3 + j]];
				res.grow(v);
			}
		}
		return res;
	}

	template<typename T>
	BoundingBoxGLM<typename T> combineBoundingBoxes(const BoundingBoxGLM<typename T>& a, const BoundingBoxGLM<typename T>& b)
	{
		auto res = a;
		res.grow(b);
		return res;
	}
};

